#include "stdafx.h"
#include "AVRConfigurator.h"
//#include "Dlg_LibProperties.h"
#include "GccWrapperAvr.h"
#include "AvrAssigned.h"
#include "VisitorTree.h"
#include "FilesContainer.h"
#include "System.h"
#include "Lib.h"
#include "Global.h"
#include "error.h"
#include "FilesContainer.h"

// Class CLibraries

CLibraries::CLibraries(CContainer* parent) :
	LazyItemListContainer(parent, LIBRARIES_ID, L"Libraries", AlphaIcon::Library) {

}

void CLibraries::ConstructMenu(CMenu& menu) noexcept {
	menu.AppendMenu(MF_STRING, ID_CREATE_LIB, _T("New Lib"));
}

void CLibraries::HandleRequest(REQUEST* req) noexcept {
	int id = -1;
	
	switch (req->ID_MENU) {
	case ID_CREATE_LIB:
		try {
			id = db->sequence_next_val<tb::tree_sequence>(3);
			LoadChilds();
			auto lib = CLib::Factory::Create<CLib>(this, id);
			AddItem(lib);
		} catch (std::exception& ex) {
			if (id != -1) {
				db->delete_from<tb::lib_t, pr::single_where>(pr::make_single_where(
					pr::base::cond<tb::lib_t, op::eq, LF::F_id>(id)
				));
			}
			MessageBoxA(0, ex.what(), "Error", MB_ICONEXCLAMATION);
		}
		break;
	default:
		__super::HandleRequest(req);
	};
}

void CLibraries::GetMap(std::map<string, CLib*>& map) {
	for (auto&l : m_items)
		map[l->GetU8Name()] = l;
}

void CLibraries::GetNameByID(std::map<int, string>& map) {
	for (auto&l : m_items) 
		map[l->GetId()] = l->GetU8Name();
}

LibObjectFile* CLibraries::GetLibraryObjectFile(const LibObject& o) {
	auto lib = GetElementById(o.GetLibId());
	ATLASSERT(lib);
	return &lib->m_library->libobjects[o.GetName()];
}

BOOST_CLASS_EXPORT_GUID(LibBlob, "LibBlob")

CLib::CLib(CContainer* parent, int id) :
	ItemListContainer(parent, id) {
	tb::lib_t lib;
	lib.id = id;
	lib.name.s = "lib_" + std::to_string(db->sequence_next_val<tb::lib_name_sequence>());

	SetName(to_utf16(lib.name.s));
	icon_ = AlphaIcon::Book;

	if (!CreateDirectoryW(global.GetLibrariesDirW() + GetName(), NULL) ||
		!CreateDirectoryW(global.GetLibrariesDirW() + GetName() + _T("\\obj"), NULL)) {
		throw std::runtime_error("could't create the directory\r\nCheck library catalog");
	}
	
	db->insert<tb::lib_t, tb::lib_t::F_id, tb::lib_t::F_name>(lib);

	insert(new CFilesContainer(this, id + 1, _T("Header Files")));
	insert(new CFilesContainer(this, id + 2, _T("Source Files")));

	m_library.create();
	m_library.save(db.get(), GetId());

	SaveChilds();
}

// From DataBase
CLib::CLib(CContainer* parent, const tb::lib_t& data) :
	ItemListContainer(parent, data.id, to_utf16(data.name.s), AlphaIcon::Book) {
	LoadChilds();
	m_library.load(db.get(), GetId());
}

INT_PTR CLib::ShowProperties() {
/*	CDlg_LibProperties dlg(m_library->additionalLib);
	auto result = dlg.DoModal();
	if (result == IDOK) {
		std::vector<int>::iterator begin = m_library->additionalLib.begin(), end = m_library->additionalLib.end(), it;
		it = std::find(begin, end, GetId());
		if (it != end) {
			m_library->additionalLib.erase(it);
		}
		m_library.save(db.get(), GetId());
	}
	return result;*/
	return 0;
}
void CLib::ConstructMenu(CMenu& menu) noexcept {
	menu.AppendMenu(MF_STRING, ID_LIB_COMPILE, _T("Build"));
	menu.AppendMenu(MF_SEPARATOR);
	CMenu addMenu;
	addMenu.CreatePopupMenu();
	addMenu.AppendMenu(MF_STRING, ID_CREATE_FILE, _T("New File"));
	addMenu.AppendMenu(MF_STRING, ID_CREATE_FILTER, _T("New Filter"));
	addMenu.AppendMenu(MF_STRING, ID_ADD_EXIST, _T("Existing Item"));
	menu.AppendMenu(MF_POPUP, (UINT_PTR)addMenu.m_hMenu, _T("Add"));
	addMenu.DestroyMenu();

	DefaultMenu(menu);
}

int CLib::Move(CFileItem* pFileItem, int action) {
	if (action) {
		return MoveBranch(pFileItem);
	}
	return 1;
}

int CLib::Move(CFilesContainer* pFileContaiiner, int action) {
	if (action) {
		MoveBranch(pFileContaiiner);
	}
	return 1;
}

void CLib::Build() {
	string stFiles;
	std::vector<string> ofiles;
	boost::regex expr;
	boost::smatch what;
	std::map<string, std::set<string>> objFunc;
	
	CFindType<CFileItem> FileItems;
	Traversal<CItem>(this, FileItems);
	
	OUTPUT_WND_PTR->ClearOutput();
	OUTPUT_WND_PTR->PrintW(L"------Build started : Library : " + GetName());
	
	for (auto& i : FileItems) {
		if (i->IsCompile() && (i->GetExtension() == FILE_TYPE::ASM || i->GetExtension() == FILE_TYPE::C)) {
			string file = i->GetU8Name();  
			ofiles.push_back(remove_ext(file) + ".o");
			stFiles += "..\\" + file + " ";
		}
	}

	CGccWrapperAvr gcc;
	gcc.SetDir(global.GetLibrariesDir() + GetU8Name() + "\\obj");
	gcc.SetMC("atmega16");
	gcc.AppendInclude("..\\..\\..\\include");
	gcc.AppendOptions("-c -nostartfiles -nodefaultlibs -nostdlib");
	if (!gcc.Compile(stFiles)) {
		CreateDependenceTree();
		for (auto& ofile : ofiles) {
			std::vector<Symbol> symbols = gcc.GetSymbolTable(ofile, "-g");
			for (auto& sym : symbols) {
				if (sym.type == 'T') {
					objFunc[ofile].emplace(sym.name);
				}
			}
		}

		std::map<string, string> objFiles;

		WIN32_FIND_DATAA FindData;
		string stDir = global.GetLibrariesDir() + GetU8Name() + "\\obj\\";
		HANDLE hFind = FindFirstFileA((stDir + "*.o").c_str(), &FindData);
		if (hFind == INVALID_HANDLE_VALUE)
			return;
		objFiles[FindData.cFileName] = stDir + FindData.cFileName;
		while (FindNextFileA(hFind, &FindData))
			objFiles[FindData.cFileName] = stDir + FindData.cFileName;
		FindClose(hFind);

		CFindType<CAVRAssignedObjectFile> asigned;
		Traversal<CItem>(g_pSystem.get(), asigned);

		auto assigned_begin = asigned.begin(), assigned_end = asigned.end();

		for (auto& file : objFiles) {
			HASHMD5 md5;
			GetFileMD5(file.second, md5);

			auto it = m_library->libobjects.find(file.first), end = m_library->libobjects.end();

			if (it != end) {
				if (md5 != m_library->libobjects[file.first].md5) {
					// Если файл изменился
					int id = m_library->libobjects[file.first].id;
					auto assigned_it = std::find_if(assigned_begin, assigned_end, [&id](CAVRAssignedObjectFile* pObj) {
						return pObj->GetObjectID() == id &&  pObj->GetStatus() == 1; });
					if (assigned_it != assigned_end) {
						(*assigned_it)->SetStatus(2);
					}
					m_library->libobjects[file.first].md5 = md5;
					// Старые функции удаляем
					m_library->libobjects[file.first].expf.clear();
					for (auto&f : objFunc[file.first])
						m_library->libobjects[file.first].expf.push_back(f);
				}
			} else {
				// Новый объектный файл
				LibObjectFile libObject;
				libObject.id = db->sequence_next_val<tb::tree_sequence>();
				libObject.idLib = GetId();
				libObject.stObjName = _wsc.from_bytes(file.first).c_str();
				libObject.md5 = md5;
				
				for (auto&f : objFunc[file.first])
					libObject.expf.push_back(f);

				m_library->libobjects[file.first] = libObject;
			}
		}
	}
	m_library.save(db.get(), GetId());
}


void CLib::CreateDependenceTree() {
	string stInput, str, stDir;
	CFindType<CLib> libs;
	Traversal<CItem>(GetParent(), libs);
	
	WIN32_FIND_DATAA FindData;
	HANDLE hFind;
	std::vector<int>::iterator begin = m_library->additionalLib.begin();
	std::vector<int>::iterator end = m_library->additionalLib.end();
	try {
		SetCurrentDirectoryW(global.GetAppRoamingDirW());
		for (auto& l : libs) {
			stDir = "..\\..\\" + l->GetU8Name() + "\\obj\\";
			SetCurrentDirectoryA((global.GetLibrariesDir() + l->GetU8Name() + "\\obj\\").c_str());
			if (std::find(begin, end, l->GetId()) != end) {
				hFind = FindFirstFileA("*.o", &FindData);
				if (hFind == INVALID_HANDLE_VALUE)
					return;
				stInput += stDir + FindData.cFileName + string(" ");
				while (FindNextFileA(hFind, &FindData))
					stInput += stDir + FindData.cFileName + string(" ");
				FindClose(hFind);
			}
		}
		// Все свои объектные файлы
		SetCurrentDirectoryA((global.GetLibrariesDir() + GetU8Name() + "\\obj\\").c_str());
		hFind = FindFirstFileA("*.o", &FindData);
		if (hFind == INVALID_HANDLE_VALUE)
			return;
		stInput += FindData.cFileName + string(" ");
		while (FindNextFileA(hFind, &FindData))
			stInput += FindData.cFileName + string(" ");
		FindClose(hFind);

		if (CreateProccessWindowlessA(global.config.RunAvrGccExe("avr-ld") + "--cref " + stInput, "", str)) {
			//
		}

		SetCurrentDirectoryW(global.GetAppRoamingDirW());
		
		boost::replace_all(str, "..\\..\\", "<");
		boost::replace_all(str, "\\obj\\", ">");
		
		// Gen Tree From Cross Table
		const char* pTable = str.c_str();
		const char* pLineBegin = pTable;
		const char* pLineEnd = pTable;

		boost::regex expr_1("^([0-9a-zA-Z_><]+)(\\s*)([0-9a-zA-Z_><]+.o)$"),
			expr_2("^(\\s*)([0-9a-zA-Z_><]+.o)$");
		boost::smatch what;

		string strCurrent;
		m_library->table.clear();
		while (*pLineBegin) {
			while (*pLineEnd && (*pLineEnd) != '\n') {
				pLineEnd++;
			}
			string strLine(pLineBegin, pLineEnd - 1);

			if (boost::regex_search(strLine, what, expr_1)) {
				strCurrent = what[3];
			} else {
				if (boost::regex_search(strLine, what, expr_2)) {
					m_library->table[what[2]].push_back(strCurrent);
				}
			}
			if (!(*pLineEnd)) break;
			pLineBegin = ++pLineEnd;
		}
#ifdef _DEBUG
		for (auto& i : m_library->table) {
			DBG_PRINT(i.first + ":\n");
			for (auto& j : i.second) {
				DBG_PRINT("    " + j + "\n");
			}
		}
#endif
	} catch (std::exception& ex) {
		OUTPUT_WND_PTR->PrintA(ex.what());
		SetCurrentDirectoryW(global.GetAppRoamingDirW());
	}
}
void CLib::Visit(CVisitor& v) {
	v.Accept(this);
}
void CLib::HandleRequest(REQUEST* req) noexcept {
	CStringW str;
	switch (req->ID_MENU) {
	case ID_CREATE_FILTER:
		
		AddItem(new CFilesContainer(this, db->sequence_next_val<tb::tree_sequence>(), GetNextName(_T("New Folder"))));
		SaveChilds();
		break;
	case ID_CREATE_FILE:
		str = global.GetLibrariesDirW() + GetName();
		if (CFileItem* pFile = CFileItem::Create(this, str)) {
			AddItem(pFile);
			SaveChilds();
		}
		break;
	case ID_ADD_EXIST:
		AddFilesToCategory(this);
		SaveChilds();
		break;
	case ID_LIB_COMPILE:
		Build();
		break;
	default:
		__super::HandleRequest(req);
	};
}
void CLib::SaveData() {

}
void CLib::PostDelete() noexcept {
	try {
		{
			using condition = prm::single_condition<tb::lib_t::F__id, op::eq>;
			using delete_from = prm::delete_from<tb::lib_t, condition>;
			db->execute_delete(delete_from(), GetId());
		}
	} catch (pqxx::pqxx_exception& e) {
		ShowException(e.base().what());
	} 
}