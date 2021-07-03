#include "stdafx.h"
#include "tr_system.h"
#include "library.h"
#include "stuff.h"
#include "gccwrapperavr.h"

npsys::LibList npsys::libs;

inline bool is_letter(char c) {
	return (c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z');
}

inline bool is_digit(char c) {
	return (c >= '0' && c <= '9');
}

inline bool is_lib_name(char c) {
	return is_letter(c) || is_digit(c) || (c == '.') || (c == '_') || (c == '-');
}

struct LibDesc {
	std::unordered_set<std::string> input;
	std::unordered_set<std::string> dep;
};

using LibList = std::unordered_map<std::string, LibDesc>;

LibList ParseLibFile(std::string_view file) {
	if (!std::filesystem::exists(file)) {
		std::cout << "dep file not found..." << std::endl;
		return {};
	}

	struct Token {
		enum TOKEN_ID {
			SEMICOLON, COLON, WORD, COMMA, UNKNOWN, END, FB_O, FB_C
		};
		TOKEN_ID id;
		std::string attr;
		size_t line;
		size_t col;
	};

	class Lexer {
	public:
		Lexer(std::string_view file) : line_(1), col_(0), token_readed_(false) {
			std::ifstream is(file);
			is.seekg(0, is.end);
			size_t length = is.tellg();
			is.seekg(0, is.beg);

			input_.resize(length + 1);
			is.read(input_.data(), length);
			ptr_ = input_.c_str();
		}

		Token GetToken() {
			if (token_readed_) {
				token_readed_ = false;
				return readed_;
			}
			return Scan();
		}

		Token ReadToken() {
			assert(token_readed_ == false);
			token_readed_ = true;
			readed_ = Scan();
			return readed_;
		}

	private:
		std::string input_;
		const char* ptr_;
		size_t line_;
		size_t col_;
		bool token_readed_;
		Token readed_;

		Token Scan() {
			char c;
			while (c = *ptr_++) {
				col_++;
				switch (c) {
				case ' ': 
					continue;
				case '\n':
					line_++;
					col_ = 0;
					continue;
				case '\t':
					col_ += 3;
					continue;
				case ';':
					return { Token::SEMICOLON, "", line_, col_ };
				case '{':
					return { Token::FB_O, "", line_, col_ };
				case '}':
					return { Token::FB_C, "", line_, col_ };
				case ':':
					return { Token::COLON, "", line_, col_ };
				case ',':
					return { Token::COMMA, "", line_, col_ };
				default:
					if (is_letter(c)) {
						size_t pos = col_;
						std::string word;
						word.append(1, c);
						while (c = *ptr_++) {
							if (is_lib_name(c)) {
								col_++;
								word.append(1, c);
							} else {
								ptr_--;
								break;
							}
						}
						return { Token::WORD, std::move(word), line_, pos };
					}
					return { Token::UNKNOWN, " ", line_, col_ };
				}
			}
			return  { Token::END, " ", line_, col_ };
		}
	};

	class Parser {
	public:
		class ParserException : public std::runtime_error {
			using _Mybase = std::runtime_error;
		public:
			explicit ParserException(size_t line, size_t col, const std::string& _Message)
				: _Mybase(_Message.c_str()), line_(line), col_(col) {
			}
			explicit ParserException(size_t line, size_t col, const char* _Message)
				: _Mybase(_Message), line_(line), col_(col) {
			}
			size_t line() const noexcept { return line_; }
			size_t col() const noexcept { return col_; }
		private:
			size_t line_;
			size_t col_;
		};

		Parser(std::string_view file) : lex_(file) {}

		void Parse(LibList& lst) {
			Token token;
			while ((token = lex_.GetToken()).id != Token::END) {
				if (token.id == Token::UNKNOWN) {
					throw ParserException(token.line, token.col + token.attr.length(),
						"unknown token");
				}
				ParseLibDesc(token, lst);
			}
		}
	private:
		Lexer lex_;

		void ParseLibDesc(Token token, LibList& lst) {
			if (token.id == Token::WORD) {
				auto lib = lst.insert({ token.attr, {} });
				if (!lib.second) {
					throw ParserException(token.line, token.col + token.attr.length(),
						"library is already defined");
				}

				auto next = lex_.GetToken();
				if (next.id != Token::FB_O) {
					throw ParserException(token.line, token.col + token.attr.length(),
						"there is no { after the lib declaration");
				}
				if ((next = lex_.GetToken()).id == Token::FB_C) return;
				do {
					if (next.id == Token::WORD) {
						auto col = lex_.GetToken();
						if (col.id != Token::COLON) {
							throw ParserException(token.line, token.col + token.attr.length(),
								"there is no column after the declaration");
						}
						if (next.attr == "i") ParseList(lib.first->second.input);
						if (next.attr == "d") ParseList(lib.first->second.dep);
					}
				} while ((next = lex_.GetToken()).id == Token::WORD);
				if (next.id != Token::FB_C) {
					throw ParserException(token.line, token.col + token.attr.length(),
						"there is no } after the lib declaration");
				}
			}
		}
		void ParseList(std::unordered_set<std::string>& lst) {
			auto token = lex_.GetToken();
			switch (token.id) {
			case Token::WORD:
			{
				auto next = lex_.ReadToken();
				if (next.id == Token::COMMA || next.id == Token::SEMICOLON) {
					if (!lst.insert(token.attr).second) {
						throw ParserException(token.line, token.col, "parser error");
					}
				} else {
					throw ParserException(next.line, next.col, "parser error");
				}
			}
			case Token::COMMA:
				ParseList(lst);
				break;
			case Token::SEMICOLON:
				return;
			default:
				throw ParserException(token.line, token.col, "parser error");
			}
		}
	};

	try {
		LibList result;
		Parser parser(file);
		parser.Parse(result);
		return std::move(result);
	} catch (Parser::ParserException& ex) {
		std::cout << "Error in Line: " << ex.line() << " Col: " << ex.col() << " " << ex.what() << std::endl;
	}

	return {};
}

std::optional<npsys::CLibrary> CreateAlphaLibrary(
	const std::string& libname, 
	const LibDesc& lib, 
	CGccWrapperAvr& compiler, 
	npsys::LIBRARY_TARGET target);

void CreateDependenceTree(
	std::vector<npsys::CLibrary>& libs,
	npsys::CLibrary& lib,
	npsys::LIBRARY_TARGET target);

void AlphaCompileLibraries() {
	auto lib_dir = global.GetLibrariesDir();
	std::error_code ec;
	std::filesystem::current_path(lib_dir, ec);

	if (ec) {
		std::cout << ec.message() << std::endl;
		return;
	}

	if (std::filesystem::exists("libs.db")) {
		std::ifstream is("libs.db", std::ios_base::binary);
		try {
			boost::archive::binary_iarchive oa(is);
			oa >> npsys::libs;
		} catch (std::exception& ex) {
			std::cout << ex.what() << std::endl;
		}
		return;
	}

	auto libs_desc = ParseLibFile("./avr/dep.txt");
	if (!libs_desc.size()) return;
	CGccWrapperAvr gcc;
	gcc.SetMC("atmega16");
	gcc.AppendOptions("-c -nostartfiles -nodefaultlibs -nostdlib");
	std::vector<npsys::CLibrary> libs;
	for (auto& lib : libs_desc) {
		auto r = CreateAlphaLibrary(lib.first, lib.second, gcc, npsys::LIBRARY_TARGET::LIBRARY_TARGET_AVR);
		if (r) libs.emplace_back(std::move(r.value()));
	}

	for (auto& lib : libs) 
		CreateDependenceTree(libs, lib, npsys::LIBRARY_TARGET::LIBRARY_TARGET_AVR);

	npsys::libs = libs;

	std::filesystem::current_path(lib_dir, ec);
	std::ofstream os("libs.db", std::ios_base::binary);
	try {
		boost::archive::binary_oarchive oa(os);
		oa << npsys::libs;
	} catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
}

inline string remove_ext(const string& str) {
	size_t pos = str.find_last_of('.');
	return string(str.c_str(), pos);
}

inline auto subdir (npsys::LIBRARY_TARGET target) {
	if (target == npsys::LIBRARY_TARGET::LIBRARY_TARGET_AVR) return "avr\\";
	return "unknown";
};

std::optional<npsys::CLibrary> CreateAlphaLibrary(
	const std::string& libname, 
	const LibDesc& lib, 
	CGccWrapperAvr& compiler, 
	npsys::LIBRARY_TARGET target) 
{
	npsys::CLibrary result;
	result.name = libname;
	result.additionalLibs = lib.dep;

	std::vector<string> ofiles;
	boost::regex expr;
	boost::smatch what;

	std::cout << "------Build started : Library : " + libname;

	std::string input;
	for (auto& i : lib.input) {
		input += "../" + i + " ";
		ofiles.push_back(remove_ext(i) + ".o");
	}

	std::string out;
	compiler.SetDir(global.GetLibrariesDir() + subdir(target) + libname + "\\obj");
	auto ok = compiler.Compile(input, out);
	std::cout << out << std::endl;
	if (ok != 0) return {};
	
	for (auto& file : ofiles) {
		auto o = result.libobjects.insert({ file, {} });
		assert(o.second);
		o.first->second.name = file;
		auto symbols = compiler.GetSymbolTable(file, "-g");
		for (auto& sym : symbols) {
			if (sym.type == 'T') {
				o.first->second.expf.push_back(sym.name);
			}
		}
	}

	return std::move(result);
}

void CreateDependenceTree(
	std::vector<npsys::CLibrary>& libs, 
	npsys::CLibrary& lib, 
	npsys::LIBRARY_TARGET target) 
{
	string input, str;
	std::filesystem::current_path(global.GetLibrariesDir() + subdir(target) + lib.name + "\\obj");
	
	for (auto& l : libs) {
		if (lib.additionalLibs.find(l.name) != lib.additionalLibs.end()) {
			for (auto& o : l.libobjects) {
				input += "../../" + l.name + "/obj/" + o.first + " ";
			}
		}
	}

	for (auto& o : lib.libobjects) input += o.first + " ";

	auto ok = CreateProccessWindowlessA(global.config.RunAvrGccExe("avr-ld") + "--cref " + input, "", str);
	std::filesystem::current_path(global.GetAppRoamingDir());
	
	boost::replace_all(str, "../../", "<");
	boost::replace_all(str, "/obj/", ">");

	// Gen Tree From Cross Table
	const char* pTable = str.c_str();
	const char* pLineBegin = pTable;
	const char* pLineEnd = pTable;

	boost::regex expr_1("^([0-9a-zA-Z_><]+)(\\s*)([0-9a-zA-Z_><]+.o)$"),
		expr_2("^(\\s*)([0-9a-zA-Z_><]+.o)$");
	boost::smatch what;

	string strCurrent;
	while (*pLineBegin) {
		while (*pLineEnd && (*pLineEnd) != '\n') {
			pLineEnd++;
		}
		string strLine(pLineBegin, pLineEnd - 1);

		if (boost::regex_search(strLine, what, expr_1)) {
			strCurrent = what[3];
		} else {
			if (boost::regex_search(strLine, what, expr_2)) {
				lib.table[what[2]].push_back(strCurrent);
			}
		}
		if (!(*pLineEnd)) break;
		pLineBegin = ++pLineEnd;
	}
/*
	for (auto& i : lib.table) {
		std::cout << i.first << ":\n";
		for (auto& j : i.second) 
			std::cout << "    " << j << "\n";
	}
	std::cout.flush();
*/
}

/*
BOOST_CLASS_EXPORT_GUID(CTreeLibraryFile, "libfile");
BOOST_CLASS_EXPORT_GUID(CTreeLibraryFilter, "libfilter");

void AddFilesToCategory(CContainer* category);

class CTreeLibraryFile : public TItem<CategoryElement> {
	using Base = TItem<CategoryElement>;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		BASE_ITEM_SERIALIZE();
		ar & path_;
		ar & file_type_;
	}
public:
	CTreeLibraryFile() {}
	CTreeLibraryFile(oid_t id, CString name, CString path, npsys::FILE_TYPE file_type)
		: Base(id, name, AlphaIcon::Binary)
		, path_(path)
		, file_type_(file_type) {
		SelectIcon();
	}
	char* OpenFile();
	void SaveFile(char* lpszText, int lenght);
	void CloseFile();
	virtual void HandleRequest(REQUEST* req) noexcept;
	//	virtual int Move(CItem*, int);
	//	static CFileItem* Create(CItem* pContainer, CString stPath);
	virtual bool ChangeName(const CString& name);
	virtual void SaveItem();
	virtual BOOL IsModified();
	//	virtual void Visit(CVisitor&);
	void SelectIcon();
	string GetWithoutExt();
	npsys::FILE_TYPE GetExtension() const noexcept { return file_type_; }
	const CString& GetPath() const noexcept { return path_; }
	bool IsCompile();
	//	CString GetPathName() { return m_stPathName; }
	DECLARE_VISITOR()
	static CTreeLibraryFile* Create(oid_t id, CString path) {
		CString name;
		name = path;
		int i = name.ReverseFind(L'\\');
		name.Delete(0, i + 1);

		CStringW ex = name;

		i = ex.ReverseFind('.');
		ex.Delete(0, i);

		npsys::FILE_TYPE file_type;
		if (ex == ".h") {
			file_type = npsys::FILE_TYPE::FILE_TYPE_HEADER;
		} else if (ex == ".S") {
			file_type = npsys::FILE_TYPE::FILE_TYPE_ASM;
		} else if (ex == ".c") {
			file_type = npsys::FILE_TYPE::FILE_TYPE_C;
		} else {
			file_type = npsys::FILE_TYPE::FILE_TYPE_UNK;
		}
		return new CTreeLibraryFile(id, name, path, file_type);
	}
	static CTreeLibraryFile* Create(CTreeItemAbstract* root_item, CString path) {
		//	CDlg_NewFile dlg(root_item, path);
		//	if (dlg.() == IDOK) {
		//		return new CFileItem(pContainer, db->sequence_next_val<tb::tree_sequence>(), stFilePath);
		//	} 
		return nullptr;
	}
private:
	CString path_;
	npsys::FILE_TYPE file_type_;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	char* m_lpszText;
	bool m_bFileOpen;
	bool m_bRemoved = false;
	bool m_bCompile;

	int IsFileExists(CString stFile);
};

IMPLEMENT_VISITOR(CTreeLibraryFile);

int CTreeLibraryFile::IsFileExists(CString stFile) {
	HANDLE  hFile = CreateFile(stFile, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		DWORD dwError = GetLastError();
		if (dwError == ERROR_FILE_NOT_FOUND) {
			return false;
		}
	}
	CloseHandle(hFile);
	return true;
}

void CTreeLibraryFile::SelectIcon() {
	if (!IsFileExists(path_)) {
		m_hIcon = global.GetIcon(AlphaIcon::File_Delete);
		m_bRemoved = true;
		return;
	}
	switch (file_type_) {
	case npsys::FILE_TYPE::FILE_TYPE_C:
		m_hIcon = global.GetIcon(AlphaIcon::File_C);
		break;
	case npsys::FILE_TYPE::FILE_TYPE_ASM:
		m_hIcon = global.GetIcon(AlphaIcon::File_S);
		break;
	case npsys::FILE_TYPE::FILE_TYPE_HEADER:
		m_hIcon = global.GetIcon(AlphaIcon::File_H);
		break;
	default:
		m_hIcon = global.GetIcon(AlphaIcon::File_U);
		break;
	}
}

//void CTreeLibraryFile::ShowWindow(bool bMakeVisible)
//{
if (m_bRemoved) {
MessageBox(*g_hMainWnd, m_stPathName + _T(" was deleted"), _T("Error"), MB_ICONERROR);
return;
}
if (!IsWindowIsExists()) {
(static_cast<CMyDocManager*>(static_cast<CTCSApp*>(AfxGetApp())->m_pDocManager))->OpenItem(this, docTypes::ItemFile, bMakeVisible);
} else
{
CMainFrame* pMainWnd = (CMainFrame*)g_hMainWnd;
pMainWnd->SelectView(pView_);
}

//}

char* CTreeLibraryFile::OpenFile() {
	FILE *fp;
	errno_t err;
	err = _wfopen_s(&fp, path_, _T("rb"));

	if (err != 0) {
		return nullptr;
	}

	long size = 0;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *buf = (char*)malloc(size + 1);

	fread_s(buf, size, 1, size, fp);

	buf[size] = '\0';

	fclose(fp);

	return buf;
}

void CTreeLibraryFile::SaveFile(char* lpszText, int lenght) {
	FILE *fp;
	_wfopen_s(&fp, path_, _T("w+b"));
	fwrite(lpszText, 1, lenght, fp);
	fclose(fp);
}

void CTreeLibraryFile::CloseFile() {

}

void CTreeLibraryFile::HandleRequest(REQUEST* req) noexcept {
	//	switch (req->ID_MENU)
	//	{
	//		default:
	__super::HandleRequest(req);
	//	}
}

int CTreeLibraryFile::Move(CItem* pItem, int action) {
	int res = pItem->Move(this, action);
	return res;
}


bool CTreeLibraryFile::ChangeName(const CString& name) {
	//CString stOldFile = m_stPathName;

	
	CString stNewFile = m_stPath + name;
	if (!IsFileExists(stOldFile)) {
	icon_ = File_Delete;
	m_bRemoved = true;
	return false;
	}
	if (IsFileExists(stNewFile))
	return false;
	if (__super::ChangeName(name))
	MoveFile(stOldFile, stNewFile);
	
	return true;
}

BOOL CTreeLibraryFile::IsModified() {
	//	if (view_)
	//	{
	//		return static_cast<CFileItemView*>(pView_)->scWnd.IsChanged();
	//	}
	return FALSE;
}

void CTreeLibraryFile::SaveItem() {

	//	if (view_)
	//	{
	//		static_cast<CFileItemView*>(pView_)->Save();
	//	}
}




bool CTreeLibraryFile::IsCompile() {
	return true;
}

string CTreeLibraryFile::GetWithoutExt() {
	int dot = GetName().ReverseFind('.');
	if (dot == -1)
		return converter.to_bytes((LPCWSTR)GetName());

	CStringW name = GetName();
	name.Delete(dot, name.GetLength());
	return converter.to_bytes((LPCWSTR)name);
}

#define BRANCH_DESTROY 234754

class CTreeLibraryFilter : public ItemListContainer<std::list, CTreeItemAbstract,
	CategoryElement, Manual> {
	using Base = ItemListContainer<std::list, CTreeItemAbstract, CategoryElement, Manual>;

	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		BASE_ITEM_SERIALIZE();
	}
public:
	CTreeLibraryFilter() {}
	CTreeLibraryFilter(oid_t id, const CString& name);
	void AddFolder(CTreeLibraryFilter* pFolder);
	void HandleRequest(REQUEST* req) noexcept;
	virtual int Move(CTreeItemAbstract*, int) final;
	virtual int Move(CTreeLibraryFile*, int) final;
	virtual int Move(CTreeLibraryFilter*, int) final;
	virtual void ConstructMenu(CMenu* menu) noexcept;

	static void AddFilter(CContainer* where, const CString& name) {
		m_treeview.Expand(where->GetHTR(), TVE_EXPAND);
		where->AddItem(
			new CTreeLibraryFilter(
				odb::Database::Get()->next_oid(1),
				name));
		where->StoreItem();
	}
};

CTreeLibraryFilter::CTreeLibraryFilter(oid_t id, const CString& name)
	: Base(id, name, AlphaIcon::Folder_Close) {
}

void CTreeLibraryFilter::AddFolder(CTreeLibraryFilter* filter) {
	insert(filter);
	filter->Insert(GetHTR());
}

void CTreeLibraryFilter::HandleRequest(REQUEST* req) noexcept {
	switch (req->ID_MENU) {
		//	case ID_ADD_FILE: {
		//	CModule* parent = reinterpret_cast<CModule*>(parent_);
		//	CString stDir;
		//	while (parent && !dynamic_cast<CModule*>(parent)) {
		//		parent = (CModule*)parent->GetParent();
		//	}
		//	if (parent != NULL) {
		//		stDir = ((CDir*)(parent))->GetDir();
		//	}
		//		break;
		//	}
	case ID_CREATE_FILTER:
		CTreeLibraryFilter::AddFilter(this, GetNextName(_T("New Folder")));
		break;
	case ID_CREATE_FILE:
		m_treeview.Expand(m_hTreeItem, TVE_EXPAND);
		req->ID_MENU = ID_CREATE_FILE;
		GetParent()->HandleRequest(req);
		break;
	case ID_ADD_EXIST:
		m_treeview.Expand(m_hTreeItem, TVE_EXPAND);
		AddFilesToCategory(this);
		StoreItem();
		break;
	default:
		__super::HandleRequest(req);
	}
}
int CTreeLibraryFilter::Move(CTreeItemAbstract* item, int action) {
	int result = item->Move(this, action);
	if (result && action) StoreItem();
	return result;
}
int CTreeLibraryFilter::Move(CTreeLibraryFile* file, int action) {
	if (action) return MoveBranch(file);
	return 1;
}

int CTreeLibraryFilter::Move(CTreeLibraryFilter* filter, int action) {
	if (action) return MoveBranch(filter);
	return 1;
}

void CTreeLibraryFilter::ConstructMenu(CMenu* menu) noexcept {
	CMenu addMenu;
	addMenu.CreatePopupMenu();
	addMenu.AppendMenu(MF_STRING, ID_CREATE_FILE, _T("New File"));
	addMenu.AppendMenu(MF_STRING, ID_CREATE_FILTER, _T("New Filter"));
	addMenu.AppendMenu(MF_STRING, ID_ADD_EXIST, _T("Existing Item"));
	menu->AppendMenu(MF_POPUP, reinterpret_cast<UINT_PTR>(addMenu.m_hMenu), _T("Add"));
	addMenu.DestroyMenu();
	DefaultMenu(menu);
}


class CTreeLibrary : public LazyItemListContainer<
	std::vector,
	CTreeItemAbstract,
	DatabaseElement<npsys::library_n>,
	DataChilds> {
public:
	CTreeLibrary(npsys::library_n n) {
		SetIcon(AlphaIcon::Library);
		n_ = n;
	}
	DECLARE_VISITOR()
	virtual void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_LIB_COMPILE, _T("Build"));
		menu->AppendMenu(MF_SEPARATOR);
		CMenu addMenu;
		addMenu.CreatePopupMenu();
		addMenu.AppendMenu(MF_STRING, ID_CREATE_FILE, _T("New File"));
		addMenu.AppendMenu(MF_STRING, ID_CREATE_FILTER, _T("New Filter"));
		addMenu.AppendMenu(MF_STRING, ID_ADD_EXIST, _T("Existing Item"));
		menu->AppendMenu(MF_POPUP, (UINT_PTR)addMenu.m_hMenu, _T("Add"));
		addMenu.DestroyMenu();
		DefaultMenu(menu);
	}

	virtual void HandleRequest(REQUEST* req) noexcept {
		CStringW str;
		switch (req->ID_MENU) {
		case ID_CREATE_FILTER:
			CTreeLibraryFilter::AddFilter(this, GetNextName(_T("New Folder")));
			break;
		case ID_CREATE_FILE:
			m_treeview.Expand(m_hTreeItem, TVE_EXPAND);
			//	str = global.GetLibrariesDirW() + GetName();
			//	if (CFileItem* pFile = CFileItem::Create(this, str)) {
			//		AddItem(pFile);
			//		SaveChilds();
			//	}
			break;
		case ID_ADD_EXIST:
			m_treeview.Expand(m_hTreeItem, TVE_EXPAND);
			AddFilesToCategory(this);
			StoreItem();
			break;
		case ID_LIB_COMPILE:
			LoadChilds();
			Build();
			break;
		default:
			__super::HandleRequest(req);
		};
	}
	void Build();
	void CreateDependenceTree();
protected:
	virtual bool LoadChildsImpl() {
		return __super::LoadChildsImpl();
	}
	virtual void PostDelete(CContainer* parent) noexcept {
		n_.remove();
	}
	virtual bool IsRemovable() {

		return true;
	}
};

IMPLEMENT_VISITOR(CTreeLibrary);


// CTreeLibraries

CTreeLibraries::CTreeLibraries() {
	SetIcon(AlphaIcon::Library);
	name_ = L"LIBRARIES";
}

void CTreeLibraries::HandleRequest(REQUEST* req) noexcept {
	switch (req->ID_MENU) {
	case ID_CREATE_LIB: {
		m_treeview.Expand(m_hTreeItem, TVE_EXPAND);
		npsys::CLibrary* lib = new npsys::CLibrary;
		lib->name = to_utf8(GetNextName("LIBRARY_"));
		auto n = libs_.add_value(lib);
		if (n) AddItem(new CTreeLibrary(n));
		break;
	}
	default:
		__super::HandleRequest(req);
	};
}

void CTreeLibraries::ConstructMenu(CMenu* menu) noexcept {
	menu->AppendMenu(MF_STRING, ID_CREATE_LIB, _T("New Library"));
}

oid_t CTreeLibraries::GetId() const noexcept {
	return npsys::LIBRARIES_ID_;
}

bool CTreeLibraries::IsRemovable() {
	return false;
}

bool CTreeLibraries::LoadChildsImpl() {
	libs_.fetch_all_nodes();
	for (auto& i : libs_) {
		insert(new CTreeLibrary(i));
	}
	return libs_.replicated();
}

void AddFilesToCategory(CContainer* category) {
	constexpr auto MAX_FILE_COUNT = 100;
	constexpr auto FILE_LIST_BUFFER_SIZE = MAX_FILE_COUNT * (MAX_PATH + 1) + 1;

	CFindType<CTreeLibraryFile> ex_files;
	Traversal<CTreeItemAbstract>(category, ex_files);

	TCHAR szFilter[] = _T("All Files (*.*)\0*.*\0(*.c)\0*.c\0(*.S)\0*.S*\0(*.h)\0*.h\0\0");

	std::unique_ptr<TCHAR[]> buffer(new TCHAR[FILE_LIST_BUFFER_SIZE]);

	buffer[0] = 0;

	CFileDialog dlg(TRUE);

	dlg.m_ofn.Flags |= OFN_ALLOWMULTISELECT;
	dlg.m_ofn.lpstrFile = buffer.get();
	dlg.m_ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;
	dlg.m_ofn.lpstrFilter = szFilter;

	if (dlg.DoModal() != IDOK)
		return;

	auto files = OPENFILENAME_Unpack(buffer.get());
	auto ex_begin = ex_files.begin(), ex_end = ex_files.end();

	auto id = odb::Database::Get()->next_oid(files.size());
	for (const auto& file : files) {
		if (std::find_if(ex_begin, ex_end, [&file](const auto item) { return item->GetPath() == file; }) != ex_end)
			continue;
		category->AddItem(CTreeLibraryFile::Create(id++, file));
	}
}
*/



