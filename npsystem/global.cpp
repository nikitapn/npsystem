// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "Stuff.h"
#include "Global.h"
#include "constants.h"
#include "tabcontainer.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

struct MYICON_INFO
{
	int     nWidth;
	int     nHeight;
	int     nBitsPerPixel;
};

MYICON_INFO MyGetIconInfo(HICON hIcon);

// =======================================

MYICON_INFO MyGetIconInfo(HICON hIcon)
{
	MYICON_INFO myinfo;
	ZeroMemory(&myinfo, sizeof(myinfo));

	ICONINFO info;
	ZeroMemory(&info, sizeof(info));

	BOOL bRes = FALSE;

	bRes = GetIconInfo(hIcon, &info);
	if (!bRes)
		return myinfo;

	BITMAP bmp;
	ZeroMemory(&bmp, sizeof(bmp));

	if (info.hbmColor)
	{
		const int nWrittenBytes = GetObject(info.hbmColor, sizeof(bmp), &bmp);
		if (nWrittenBytes > 0)
		{
			myinfo.nWidth = bmp.bmWidth;
			myinfo.nHeight = bmp.bmHeight;
			myinfo.nBitsPerPixel = bmp.bmBitsPixel;
		}
	} else if (info.hbmMask)
	{
		// Icon has no color plane, image data stored in mask
		const int nWrittenBytes = GetObject(info.hbmMask, sizeof(bmp), &bmp);
		if (nWrittenBytes > 0)
		{
			myinfo.nWidth = bmp.bmWidth;
			myinfo.nHeight = bmp.bmHeight / 2;
			myinfo.nBitsPerPixel = 1;
		}
	}

	if (info.hbmColor)
		DeleteObject(info.hbmColor);
	if (info.hbmMask)
		DeleteObject(info.hbmMask);

	return myinfo;
}

Global global;

void Global::init() noexcept {
	{
		std::wstring temp;
		PWSTR ppszPath;
		HR(SHGetKnownFolderPath(FOLDERID_RoamingAppData, NULL, NULL, &ppszPath));
		temp = ppszPath;
		CoTaskMemFree(ppszPath);
		temp += L"\\npsystem";
		SHCreateDirectory(NULL, temp.c_str());
		app_roaming_dir_ = narrow(temp);
	}
	
	controls_state_path_ = app_roaming_dir_ + "\\controls.bin";
	config_path_ = app_roaming_dir_ + "\\config.xml";

	// asci
	std::string exe;
	char buf[260];
	::GetModuleFileNameA(NULL, buf, MAX_PATH);
	::PathRemoveFileSpecA(buf);

	module_dir_ = buf;

	alg_dir_ = app_roaming_dir_ + "\\algorithms\\";
	lib_dir_ = app_roaming_dir_ + "\\libraries\\";
	build_dir_ = app_roaming_dir_ + "\\build\\";

	{
		wchar_t buffer[MAX_PATH];
		::GetModuleFileName(NULL, buffer, MAX_PATH);
		::PathRemoveFileSpec(buffer);
		module_dir_ = narrow(buffer);
	}

	alg_dir_ = app_roaming_dir_ + "\\algorithms\\";
	lib_dir_ = app_roaming_dir_ + "\\libraries\\";
	build_dir_ = app_roaming_dir_ + "\\build\\";

	SHCreateDirectory(NULL, wide(alg_dir_).c_str());
	SHCreateDirectory(NULL,  wide(lib_dir_).c_str());

	controls.Load();

	ChangeCurrentDirectory(CurrentDirectory::ROOT);
	// GDI

	ZeroMemory(m_icons, sizeof(m_icons));
	ZeroMemory(m_cursors, sizeof(m_cursors));
	
	auto load_icon = [](int id, int cx = constants::treeview::icon_cx, int cy =  constants::treeview::icon_cy) {
		auto hIcon = (HICON)LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(id), IMAGE_ICON, cx, cy,
			LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
		ASSERT(hIcon);
		return hIcon;
	};

	m_icons[static_cast<size_t>(AlphaIcon::Pinned)] = load_icon(IDI_PINNED, 9, 9);
	m_icons[static_cast<size_t>(AlphaIcon::UnPinned)] = load_icon(IDI_UNPINNED, 9, 9);
	

	m_icons[static_cast<size_t>(AlphaIcon::Exclam)] = load_icon(IDI_EXCLAM, 12, 12);
	m_icons[static_cast<size_t>(AlphaIcon::Empty)] = load_icon(IDI_EMPTY, 12, 12);
	m_icons[static_cast<size_t>(AlphaIcon::Question)] = load_icon(IDI_NP_QUESTION, 12, 12);

	m_icons[static_cast<size_t>(AlphaIcon::Root)] = load_icon(IDI_SYSTEM);
	m_icons[static_cast<size_t>(AlphaIcon::Modules)] = load_icon(IDI_MODULES);
	m_icons[static_cast<size_t>(AlphaIcon::Module)] = load_icon(IDI_MODULE);
	m_icons[static_cast<size_t>(AlphaIcon::I2C)] = load_icon(IDI_I2C);
	m_icons[static_cast<size_t>(AlphaIcon::Container)] = load_icon(IDI_CONTAINER);
	m_icons[static_cast<size_t>(AlphaIcon::Block)] = load_icon(IDI_BLOCK);

	m_icons[static_cast<size_t>(AlphaIcon::Controller_nl)] = load_icon(IDI_CTRL_NL);
	m_icons[static_cast<size_t>(AlphaIcon::Controller_good)] = load_icon(IDI_CTRL_GOOD);
	m_icons[static_cast<size_t>(AlphaIcon::Controller_bad)] = load_icon(IDI_CTRL_BAD);
	m_icons[static_cast<size_t>(AlphaIcon::Algorithm)] = load_icon(IDI_ALGORITHM);


	m_icons[static_cast<size_t>(AlphaIcon::Hardware)] = load_icon(IDI_HARDWARE);
	m_icons[static_cast<size_t>(AlphaIcon::Software)] = load_icon(IDI_SOFTWARE);
	m_icons[static_cast<size_t>(AlphaIcon::Io)] = load_icon(IDI_IO);
	m_icons[static_cast<size_t>(AlphaIcon::Folder_Open)] = load_icon(IDI_FOLDER_OPEN);
	m_icons[static_cast<size_t>(AlphaIcon::Folder_Close)] = load_icon(IDI_FOLDER_CLOSE);
	
	m_icons[static_cast<size_t>(AlphaIcon::File_U)] = load_icon(IDI_FILE_ICON);
	m_icons[static_cast<size_t>(AlphaIcon::File_C)] = load_icon(IDI_C_FILE);
	m_icons[static_cast<size_t>(AlphaIcon::File_S)] = load_icon(IDI_S_FILE);
	m_icons[static_cast<size_t>(AlphaIcon::File_H)] = load_icon(IDI_H_FILE);
	m_icons[static_cast<size_t>(AlphaIcon::File_Delete)] = load_icon(IDI_FILE_DELETE);
	m_icons[static_cast<size_t>(AlphaIcon::Di_Pin)] = load_icon(IDI_DI_PIN);
	m_icons[static_cast<size_t>(AlphaIcon::Do_Pin)] = load_icon(IDI_DO_PIN);
	m_icons[static_cast<size_t>(AlphaIcon::Ai_Pin)] = load_icon(IDI_AI_PIN);
	m_icons[static_cast<size_t>(AlphaIcon::Ctrl_Dev)] = load_icon(IDI_CTRL_DEV);
	m_icons[static_cast<size_t>(AlphaIcon::Library)] = load_icon(IDI_LIBRARY);
	m_icons[static_cast<size_t>(AlphaIcon::Book)] = load_icon(IDI_BOOK);
	m_icons[static_cast<size_t>(AlphaIcon::Fun)] = load_icon(IDI_FUN);
	m_icons[static_cast<size_t>(AlphaIcon::Fun_cat)] = load_icon(IDI_FUNCTIONS);
	m_icons[static_cast<size_t>(AlphaIcon::Source)] = load_icon(IDI_SOURCE);
	m_icons[static_cast<size_t>(AlphaIcon::Object)] = load_icon(IDI_BINARY);
	m_icons[static_cast<size_t>(AlphaIcon::PC)] = load_icon(IDI_PC);
	m_icons[static_cast<size_t>(AlphaIcon::Network)] = load_icon(IDI_NETWORK);
	m_icons[static_cast<size_t>(AlphaIcon::Var)] = load_icon(IDI_VAR);
	m_icons[static_cast<size_t>(AlphaIcon::Binary)] = load_icon(IDI_BINARY);
	

	m_cursors[static_cast<size_t>(AlphaCursor::Arrow)].LoadSysCursor(IDC_ARROW);
	m_cursors[static_cast<size_t>(AlphaCursor::Block)].LoadSysCursor(IDC_NO);
	m_cursors[static_cast<size_t>(AlphaCursor::Drag)].LoadSysCursor(IDC_SIZEALL);
	m_cursors[static_cast<size_t>(AlphaCursor::NoDrop)].LoadSysCursor(IDC_NO);
	m_cursors[static_cast<size_t>(AlphaCursor::Hand)].LoadSysCursor(IDC_HAND);
	m_cursors[static_cast<size_t>(AlphaCursor::SizeWE)].LoadSysCursor(IDC_SIZEWE);

	m_cursors[static_cast<size_t>(AlphaCursor::Drop)].LoadCursor(IDC_DROP);
	m_cursors[static_cast<size_t>(AlphaCursor::Pen)].LoadCursor(IDC_PEN);
	m_cursors[static_cast<size_t>(AlphaCursor::LinkSelect)].LoadCursor(IDC_LINKSEL);


//	auto handle = LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(IDI_ALGORITHM), IMAGE_ICON, 24, 24, 
//		LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
//	m_icons[AlphaIcon::Algorithm] = (HICON)handle;
//	auto info = MyGetIconInfo((HICON)handle);
//	int cx = GetSystemMetrics(SM_CXICON);
//	int i = 0;
}

void Global::ChangeCurrentDirectory(CurrentDirectory dir) const noexcept {
	switch (dir)
	{
	case CurrentDirectory::ROOT:
		SetCurrentDirectory(wide(app_roaming_dir_).c_str());
		break;
	case CurrentDirectory::MODULE:
		SetCurrentDirectory(wide (module_dir_) .c_str());
		break;
	case CurrentDirectory::ALGORITHMS:
		SetCurrentDirectory(wide(alg_dir_).c_str());
		break;
	case CurrentDirectory::BUILD:
		SetCurrentDirectory(wide(build_dir_).c_str());
		break;
	case CurrentDirectory::LIBRARIES:
		SetCurrentDirectory(wide(lib_dir_).c_str());
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}

void Global::CControlsState::Default() {
	fullscreen = false;
	rc_main_wnd = { 0, 0, 1024, 768 };

	auto set_default_dock_panel_state = [] (Dock dock, bool pinned, int splitter_cxy) {
		CMyTabContainer::states[static_cast<size_t>(dock)] = { pinned, splitter_cxy };
	};

	set_default_dock_panel_state(Dock::Left, true, 400);
	set_default_dock_panel_state(Dock::Bottom, false, 300);
	set_default_dock_panel_state(Dock::Right, false, 500);

	auto set_default_dock_window_state = [] (DockIndex dock_index, Dock dock, bool created, int unpinned_cxy) {
		CDockableWindow::dockable_windows[static_cast<size_t>(dock_index)] = { dock, created, unpinned_cxy };
	};

	set_default_dock_window_state(DockIndex::SystemTree, Dock::Left, true, 400);
	set_default_dock_window_state(DockIndex::Output, Dock::Bottom, true, 200);
	set_default_dock_window_state(DockIndex::PropertyGrid, Dock::Right, true, 400);

	set_default_dock_window_state(DockIndex::Reserved0, Dock::Bottom, true, 400);
	set_default_dock_window_state(DockIndex::Reserved1, Dock::Bottom, true, 400);
	set_default_dock_window_state(DockIndex::Reserved2, Dock::Bottom, true, 400);
	set_default_dock_window_state(DockIndex::Reserved3, Dock::Bottom, true, 400);
	set_default_dock_window_state(DockIndex::Reserved4, Dock::Bottom, true, 400);
	set_default_dock_window_state(DockIndex::Reserved5, Dock::Bottom, true, 400);
	set_default_dock_window_state(DockIndex::Reserved6, Dock::Bottom, true, 400);
}

void Global::CControlsState::Load() {
	try {
		std::ifstream is(global.ControlsStatePath(), std::ios_base::binary);
		{
			boost::archive::binary_iarchive ia(is);
			ia >> *this;
		}
	} catch (...) {
		tree_state.clear();
		Default();
	}
}

void Global::CControlsState::Save() {
	for (size_t i = 0; i < CMyTabContainer::states_ptr.size(); ++i) {
		if (auto ptr = CMyTabContainer::states_ptr[i]; ptr) {
			CMyTabContainer::states[i] = ptr->GetStoredState();
		}
	}
	for (size_t i = 0; i < CDockableWindow::dockable_windows_ptr.size(); ++i) {
		if (auto ptr = CDockableWindow::dockable_windows_ptr[i]; ptr) {
			CDockableWindow::dockable_windows[i] = ptr->m_state;
		}
	}
	try {
		std::ofstream os(global.ControlsStatePath(), std::ios_base::binary);
		{
			boost::archive::binary_oarchive oa(os);
			oa << *this;
		}
	} catch (...) {}
}