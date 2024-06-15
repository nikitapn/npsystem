// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "generated/Ribbon.h"
#include "resource.h"
#include "View.h"
#include "dlgabout.h"
#include "MyTreeView.h"
#include "MyTabView.h"
#include "tabcontainer.h"
#include "dlgdoclist.h"
#include "MyStatusBar.h"
#include "PropertyGrid.h"
#include "MainFrm.h"
#include "outputwnd.h"
#include "error.h"
#include "Global.h"
#include "GDIGlobal.h"
#include <npsys/corba.h>
#include "config.h"
#include <nplib/utils/log.h>
#include <npdb/db.h>
#include <avr_info/avr_info.h>
#include <nplib/utils/thread_pool.hpp>
#include <nplib/windows/console.hpp>


constexpr bool use_console = true;

/*
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>


static void init_log() {
	namespace logging = boost::log;
	namespace sinks = boost::log::sinks;
	namespace keywords = boost::log::keywords;

#ifdef NDEBUG
	logging::add_file_log(
		keywords::file_name = "npsystem_%N.log",
    keywords::rotation_size = 10 * 1024 * 1024,
    keywords::format = "[%TimeStamp%] [%Severity%]: %Message%"
	);
#endif

	logging::core::get()->set_filter
	(
#ifdef NDEBUG
		logging::trivial::severity >= logging::trivial::warning
#else
		logging::trivial::severity >= logging::trivial::trace
#endif
	);
	logging::add_common_attributes();
}
*/

using thread_pool = nplib::thread_pool<0, 2, 0>;

CAppModule _Module;
std::unique_ptr<CMainFrame> wndMain;
HWND g_hMainWnd;
HWND g_hMainWndClient;
CProgressBarCtrl* g_progress;
CPropertyGrid* g_pPropertyGrid;
RibbonState g_ribbonState;
npsystem::Config g_cfg;
UINT g_clf_blocks;

std::thread::id g_main_thread_id;

void RibbonState::ResetBlockGallery() {
	cursor_selected = true;
	block_selected = -1;
	wndMain->m_ribbonFBDBlocks.Select(-1, true);
	wndMain->m_ribbonSFCBlocks.Select(-1, true);
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT) {
	g_main_thread_id = std::this_thread::get_id();

	wndMain = std::make_unique<CMainFrame>();
	
	if constexpr (use_console) {
		nplib::win::create_console("debug", 1024, 480);
	} else {
		std::cout.rdbuf(&wndMain->m_wndOutput);
		std::cerr.rdbuf(&wndMain->m_wndOutput);
	}

	std::cout << g_cfg << std::endl;

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	auto& rect = global.controls.rc_main_wnd;
	if (rect.left < 0 || rect.top < 0 || rect.Width() < 100 || rect.Height() < 100) {
		rect = CWindow::rcDefault;
	}

	if(wndMain->CreateEx(NULL, rect) == NULL) {
		ATLTRACE("Main window creation failed!\n");
		return 0;
	}

	nCmdShow = global.controls.fullscreen ? SW_SHOWMAXIMIZED : SW_SHOWDEFAULT;

	wndMain->ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

void ClipboardRegister() {
	g_clf_blocks = ::RegisterClipboardFormat(L"alg_blocks");
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

//	init_log();
	g_cfg.load("npsystem");
	global.init();

	avrinfo::AVRInfo::SetDataPath(nplib::config::GetExecutableRootPath() / "data");

	auto hDll = LoadLibrary(TEXT("Msftedit.dll"));
	ASSERT(hDll);
	
	auto hDllScintilla = LoadLibrary(TEXT("SciLexer.dll"));
	ASSERT(hDllScintilla);

	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	ClipboardRegister();

	bool failed = false;
	try {
		NPRPC_System::init(thread_pool::get_instance().ctx(), 
			21500,
			g_cfg.nameserver_ip,
			global.GetAppRoamingDir() + "\\keys", "npsystem",
			g_cfg.server_timeout_sec, 
			[]() { PostMessage(g_hMainWnd, WM_UPDATE_STATUS_BAR, 0, 0); }
		);
	} catch (nprpc::Exception& ex) {
		MessageBoxA(NULL, ex.what(), "npsystem error", MB_ICONERROR);
		failed = true;
	} catch (std::exception &ex) {
		MessageBoxA(NULL, ex.what(), "npsystem error", MB_ICONERROR);
		failed = true;
	}

	int nRet = -1;

	if (!failed) {
		// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
		::DefWindowProc(NULL, 0, 0, 0L);

		AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

		hRes = _Module.Init(NULL, hInstance);
		ATLASSERT(SUCCEEDED(hRes));

		nRet = Run(lpstrCmdLine, nCmdShow);
	}

	_Module.Term();

	thread_pool::get_instance().stop();

	npsys_rpc->destroy();
	::CoUninitialize();

#ifdef DEBUG
	odb::check_memory_leak();
#endif

	FreeLibrary(hDll);
	FreeLibrary(hDllScintilla);

	return nRet;
}