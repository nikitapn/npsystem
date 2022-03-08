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

	ZeroMemory(m_cursors, sizeof(m_cursors));
	
	auto load_icon = [](int id) -> std::tuple<HICON, HICON> {
		auto hIcon_24x24 = (HICON)LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(id), IMAGE_ICON, 
			constants::treeview::icon_cx, constants::treeview::icon_cy, LR_DEFAULTCOLOR);
		ASSERT(hIcon_24x24);

		auto hIcon_32x32 = (HICON)LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(id), IMAGE_ICON, 
			32, 32, LR_DEFAULTCOLOR);
		ASSERT(hIcon_32x32);

		return {hIcon_24x24, hIcon_32x32};
	};

	auto load_icon_custom_size = [](int id, int cx, int cy) -> std::tuple<HICON, HICON> {
		auto hIcon = (HICON)LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(id), IMAGE_ICON, 
			cx, cy, LR_DEFAULTCOLOR);
		ASSERT(hIcon);

		return {hIcon, NULL};
	};

	m_icons[static_cast<size_t>(NPSystemIcon::Pinned)].Attach(load_icon_custom_size(IDI_PINNED, 9, 9));
	m_icons[static_cast<size_t>(NPSystemIcon::UnPinned)].Attach(load_icon_custom_size(IDI_UNPINNED, 9, 9));
	

	m_icons[static_cast<size_t>(NPSystemIcon::Exclam)].Attach(load_icon_custom_size(IDI_EXCLAM, 12, 12));
	m_icons[static_cast<size_t>(NPSystemIcon::Empty)].Attach(load_icon_custom_size(IDI_EMPTY, 12, 12));
	m_icons[static_cast<size_t>(NPSystemIcon::Question)].Attach(load_icon_custom_size(IDI_NP_QUESTION, 12, 12));

	m_icons[static_cast<size_t>(NPSystemIcon::Root)].Attach(load_icon(IDI_SYSTEM));
	m_icons[static_cast<size_t>(NPSystemIcon::Modules)].Attach(load_icon(IDI_MODULES));
	m_icons[static_cast<size_t>(NPSystemIcon::Module)].Attach(load_icon(IDI_MODULE));
	m_icons[static_cast<size_t>(NPSystemIcon::I2C)].Attach(load_icon(IDI_I2C));
	m_icons[static_cast<size_t>(NPSystemIcon::Container)].Attach(load_icon(IDI_CONTAINER));
	m_icons[static_cast<size_t>(NPSystemIcon::Block)].Attach(load_icon(IDI_BLOCK));

	m_icons[static_cast<size_t>(NPSystemIcon::Controller_nl)].Attach(load_icon(IDI_CTRL_NL));
	m_icons[static_cast<size_t>(NPSystemIcon::Controller_good)].Attach(load_icon(IDI_CTRL_GOOD));
	m_icons[static_cast<size_t>(NPSystemIcon::Controller_bad)].Attach(load_icon(IDI_CTRL_BAD));
	m_icons[static_cast<size_t>(NPSystemIcon::Algorithm)].Attach(load_icon(IDI_ALGORITHM));


	m_icons[static_cast<size_t>(NPSystemIcon::Hardware)].Attach(load_icon(IDI_HARDWARE));
	m_icons[static_cast<size_t>(NPSystemIcon::Software)].Attach(load_icon(IDI_SOFTWARE));
	m_icons[static_cast<size_t>(NPSystemIcon::Io)].Attach(load_icon(IDI_IO));
	m_icons[static_cast<size_t>(NPSystemIcon::Folder_Open)].Attach(load_icon(IDI_FOLDER_OPEN));
	m_icons[static_cast<size_t>(NPSystemIcon::Folder_Close)].Attach(load_icon(IDI_FOLDER_CLOSE));
	
	m_icons[static_cast<size_t>(NPSystemIcon::File_U)].Attach(load_icon(IDI_FILE_ICON));
	m_icons[static_cast<size_t>(NPSystemIcon::File_C)].Attach(load_icon(IDI_C_FILE));
	m_icons[static_cast<size_t>(NPSystemIcon::File_S)].Attach(load_icon(IDI_S_FILE));
	m_icons[static_cast<size_t>(NPSystemIcon::File_H)].Attach(load_icon(IDI_H_FILE));
	m_icons[static_cast<size_t>(NPSystemIcon::File_Delete)].Attach(load_icon(IDI_FILE_DELETE));
	m_icons[static_cast<size_t>(NPSystemIcon::Di_Pin)].Attach(load_icon(IDI_DI_PIN));
	m_icons[static_cast<size_t>(NPSystemIcon::Do_Pin)].Attach(load_icon(IDI_DO_PIN));
	m_icons[static_cast<size_t>(NPSystemIcon::Ai_Pin)].Attach(load_icon(IDI_AI_PIN));
	m_icons[static_cast<size_t>(NPSystemIcon::Ctrl_Dev)].Attach(load_icon(IDI_CTRL_DEV));
	m_icons[static_cast<size_t>(NPSystemIcon::Library)].Attach(load_icon(IDI_LIBRARY));
	m_icons[static_cast<size_t>(NPSystemIcon::Book)].Attach(load_icon(IDI_BOOK));
	m_icons[static_cast<size_t>(NPSystemIcon::Fun)].Attach(load_icon(IDI_FUN));
	m_icons[static_cast<size_t>(NPSystemIcon::Fun_cat)].Attach(load_icon(IDI_FUNCTIONS));
	m_icons[static_cast<size_t>(NPSystemIcon::Source)].Attach(load_icon(IDI_SOURCE));
	m_icons[static_cast<size_t>(NPSystemIcon::Object)].Attach(load_icon(IDI_BINARY));
	m_icons[static_cast<size_t>(NPSystemIcon::PC)].Attach(load_icon(IDI_PC));
	m_icons[static_cast<size_t>(NPSystemIcon::Network)].Attach(load_icon(IDI_NETWORK));
	m_icons[static_cast<size_t>(NPSystemIcon::Var)].Attach(load_icon(IDI_VAR));
	m_icons[static_cast<size_t>(NPSystemIcon::Binary)].Attach(load_icon(IDI_BINARY));
	m_icons[static_cast<size_t>(NPSystemIcon::FBD)].Attach(load_icon(IDI_FBD));
	m_icons[static_cast<size_t>(NPSystemIcon::SFC)].Attach(load_icon(IDI_SFC));
	
	// cursors
	m_cursors[static_cast<size_t>(NPSystemCursor::Arrow)].LoadSysCursor(IDC_ARROW);
	m_cursors[static_cast<size_t>(NPSystemCursor::Block)].LoadSysCursor(IDC_NO);
	m_cursors[static_cast<size_t>(NPSystemCursor::Drag)].LoadSysCursor(IDC_SIZEALL);
	m_cursors[static_cast<size_t>(NPSystemCursor::NoDrop)].LoadSysCursor(IDC_NO);
	m_cursors[static_cast<size_t>(NPSystemCursor::Hand)].LoadSysCursor(IDC_HAND);
	m_cursors[static_cast<size_t>(NPSystemCursor::SizeWE)].LoadSysCursor(IDC_SIZEWE);
	m_cursors[static_cast<size_t>(NPSystemCursor::Drop)].LoadCursor(IDC_DROP);
	m_cursors[static_cast<size_t>(NPSystemCursor::Pen)].LoadCursor(IDC_PEN);
	m_cursors[static_cast<size_t>(NPSystemCursor::LinkSelect)].LoadCursor(IDC_LINKSEL);

//	auto handle = LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(IDI_ALGORITHM), IMAGE_ICON, 24, 24, 
//		LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
//	m_icons[NPSystemIcon::Algorithm] = (HICON)handle;
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