// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <atlribbon.h>
#include "ribbonstate.h"
#include "wm_user.h"
#include "outputwnd.h"

extern HWND g_hMainWndClient;

class CMainFrameClient : public CWindowImpl<CMainFrameClient, CWindow> {
public:
	CWindow m_wndClient;
	CMyTabContainer* m_tabContainerList;

	CMainFrameClient(CMyTabContainer& tabContainerList)
		: m_tabContainerList(&tabContainerList)
	{
	}

	DECLARE_WND_CLASS(L"CMainFrameClient")

	BEGIN_MSG_MAP(CMainFrameClient)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP()

protected:
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		UINT cx = LOWORD(lParam), cy = HIWORD(lParam);
		if (m_wndClient) m_wndClient.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);

		m_tabContainerList[0].UpdateLayoutUnpinned(cx, cy);
		m_tabContainerList[1].UpdateLayoutUnpinned(cx, cy);
		m_tabContainerList[2].UpdateLayoutUnpinned(cx, cy);

		return 0;
	}

	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { 
		return TRUE; 
	}
};

class CMainFrame :
	public CRibbonFrameWindowImpl<CMainFrame>,
	public CMessageFilter, public CIdleHandler
{
	static constexpr auto fbd_blockCount = (ID_CREATE_FBD_BLOCK_LAST - ID_GALLERY_FBD_BLOCKS - 1);
	static constexpr auto fbd_blockCatCount = 5;

	static constexpr auto sfc_blockCount = (ID_CREATE_SFC_BLOCK_LAST - ID_GALLERY_SFC_BLOCKS - 1);
	static constexpr auto sfc_blockCatCount = 1;

	enum TimerId {
		TMR_1S
	};

public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)
	CMainFrameClient m_wndClient;
	CMySplitterBottom m_splitterHor;
	CMySplitterLeft m_splitterLeft;
	CMySplitterRight m_splitterRight;

	CMyTabView m_tabview;
	CMyTreeView m_treeview{ m_tabview };

	CCommandBarCtrl m_CmdBar;
	CMyStatusBar m_statusBar;
	CPropertyGrid m_propertyGrid;

	CMyTabContainer m_tabContainerList[3];

	CMyTabContainer& m_tabContainerLeft = m_tabContainerList[0];
	CMyTabContainer& m_tabContainerBottom = m_tabContainerList[1];
	CMyTabContainer& m_tabContainerRight = m_tabContainerList[2];
	COutputEdit m_wndOutput;
	
	CRibbonItemGalleryCtrl<ID_GALLERY_FBD_BLOCKS, fbd_blockCount, fbd_blockCatCount> m_ribbonFBDBlocks;
	CRibbonItemGalleryCtrl<ID_GALLERY_SFC_BLOCKS, sfc_blockCount, sfc_blockCatCount> m_ribbonSFCBlocks;

	// Ribbon control map
	BEGIN_RIBBON_CONTROL_MAP(CMainFrame)
		RIBBON_CONTROL(m_ribbonFBDBlocks)
		RIBBON_CONTROL(m_ribbonSFCBlocks)
	END_RIBBON_CONTROL_MAP()

	virtual BOOL PreTranslateMessage(MSG* pMsg) final {
		if ((m_hAccel != NULL) && TranslateAccelerator(pMsg->hwnd, m_hAccel, pMsg))
			return TRUE;

		// filter messages from the ribbon
		return m_tabview.PreTranslateMessage(pMsg);
	}

	CMainFrame() 
		: m_tabContainerList{ Dock::Left, Dock::Bottom, Dock::Right }
		, m_wndClient { m_tabContainerList[0] }
	{
		{
			UINT uItem = 0, uCat = 0;;
			auto set_cat = [this, &uItem, &uCat](const char* text, UINT nItems) {
				m_ribbonFBDBlocks.SetCategoryText(uCat, wide(text).c_str());
				for (auto begin = uItem; uItem < begin + nItems; uItem++) {
					m_ribbonFBDBlocks.SetItemCategory(uItem, uCat);
				}
				uCat++;
			};
			set_cat("Input/Output", 2);
			set_cat("Logic", 14);
			set_cat("Math", 6);
			set_cat("Comparators", 2);
			set_cat("Control", 1);

			m_ribbonFBDBlocks.Select(-1);
			m_ribbonFBDBlocks.SetItemCategory(0, 0);
		}

		m_ribbonSFCBlocks.SetCategoryText(0, L"Basic SFC Blocks");
		//m_ribbonSFCBlocks.SetItemCategory(0, 0);
		m_ribbonSFCBlocks.Select(-1);
		m_ribbonSFCBlocks.SetItemCategory(0, 0);
	}

	CMyTabContainer* GetTabContainer(Dock dock) noexcept {
		auto index = static_cast<std::size_t>(dock);
		if (index > 2) return nullptr;
		return &m_tabContainerList[index];
	}

	virtual BOOL OnIdle() final {
		UpdateUIAll();
		UIUpdateToolBar();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_TREEPANE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_FILE_SAVE, UPDUI_DISABLED)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_PANEL_ATTACHED, OnPanelAttached)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_UPDATE_STATUS_BAR, OnStatusBarUpdate)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_TABVIEW_FOCUS, OnTabViewItemFocusSet)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_VIEW_RIBBON, OnViewRibbon)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		
		COMMAND_ID_HANDLER(ID_VIEW_SYSTEM_TREE, OnCheckViewSystemTree)
		COMMAND_ID_HANDLER(ID_VIEW_OUTPUT, OnCheckViewOutput)
		COMMAND_ID_HANDLER(ID_VIEW_PROPERTY_GRID, OnCheckViewPropertyGrid)
		
		//		COMMAND_ID_HANDLER(ID_VIEW_TREEPANE, OnViewTreePane)
		//		COMMAND_ID_HANDLER(ID_PANE_CLOSE, OnTreePaneClose)
		COMMAND_ID_HANDLER(ID_SELECT_CURSOR, OnSelectCursor)
		RIBBON_GALLERY_CONTROL_HANDLER(ID_GALLERY_FBD_BLOCKS, OnGalleryBlocks)
		RIBBON_GALLERY_CONTROL_HANDLER(ID_GALLERY_SFC_BLOCKS, OnGallerySFCBlocks)
		CHAIN_COMMANDS_MEMBER(m_tabview)
		CHAIN_MSG_MAP(CRibbonFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	PROPVARIANT m_propertyTrue;
	PROPVARIANT m_propertyFalse;

	void InitVariables() {
		m_propertyTrue.vt = VT_BOOL;
		m_propertyTrue.boolVal = -1;

		m_propertyFalse.vt = VT_BOOL;
		m_propertyFalse.boolVal = 0;
	}

	void DockWindow(CDockableWindow& wnd, bool show_window = true) noexcept {
		if (auto tab = GetTabContainer(wnd.GetDock()); tab) {
			tab->DockWindow(wnd, show_window);
		}
	};

	LRESULT OnGalleryBlocks(UI_EXECUTIONVERB verb, WORD wID, UINT uSel, BOOL& bHandled) {
		WORD id = wID + static_cast<WORD>(uSel) + 1;

		// UISetText(ID_DEFAULT_PANE, LPCWSTR(MAKEINTRESOURCE(id)));

		if (verb == UI_EXECUTIONVERB_EXECUTE) {
			g_ribbonState.cursor_selected = false;
			g_ribbonState.block_selected = id;
			m_ribbonFBDBlocks.SetImage(
				UI_PKEY_LargeImage, OnRibbonQueryItemImage(wID, uSel), true);
			m_ribbonFBDBlocks.Select(uSel, true);
		}

		return 0;
	}

	LRESULT OnGallerySFCBlocks(UI_EXECUTIONVERB verb, WORD wID, UINT uSel, BOOL& bHandled) {
		WORD id = wID + static_cast<WORD>(uSel) + 1;

		// UISetText(ID_DEFAULT_PANE, LPCWSTR(MAKEINTRESOURCE(id)));

		if (verb == UI_EXECUTIONVERB_EXECUTE) {
			g_ribbonState.cursor_selected = false;
			g_ribbonState.block_selected = id;
			m_ribbonSFCBlocks.SetImage(
				UI_PKEY_LargeImage, OnRibbonQueryItemImage(wID, uSel), true);
			m_ribbonSFCBlocks.Select(uSel, true);
		}

		return 0;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		g_hMainWnd = m_hWnd;

		InitVariables();
		bool bRibbonUI = RunTimeHelper::IsRibbonUIAvailable();
		if (bRibbonUI) {
			UIAddMenu(GetMenu(), true);
		} else {
			CMenuHandle(GetMenu()).DeleteMenu(ID_VIEW_RIBBON, MF_BYCOMMAND);
		}

		// create command bar window
		HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
		// attach menu
		m_CmdBar.AttachMenu(GetMenu());
		// load command bar images
		m_CmdBar.LoadImages(IDR_MAINFRAME);
		// remove old menu
		SetMenu(NULL);

		HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

		CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
		AddSimpleReBarBand(hWndCmdBar);
		AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

		// create status bar window
		m_hWndStatusBar = m_statusBar.Create(m_hWnd);

		::SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, FALSE, NULL, SPIF_UPDATEINIFILE);

		g_hMainWndClient = m_hWndClient = m_wndClient.Create(m_hWnd, rcDefault, NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, SPI_GETDRAGFULLWINDOWS);

		// splitter hor
		m_wndClient.m_wndClient = m_splitterHor.Create(m_hWndClient, rcDefault, NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, SPI_GETDRAGFULLWINDOWS);

		HWND tabContainerWrapperBottom = m_tabContainerBottom.Create(m_splitterHor);

		// splitter vert first left
		m_splitterLeft.Create(m_splitterHor, rcDefault, NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, SPI_GETDRAGFULLWINDOWS);

		HWND tabContainerWrapperLeft = m_tabContainerLeft.Create(m_splitterLeft);

		m_splitterRight.Create(m_splitterLeft, rcDefault, NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, SPI_GETDRAGFULLWINDOWS);

		HWND tabContainerWrapperRight = m_tabContainerRight.Create(m_splitterRight);

		// splitter vert second
		m_tabview.Create(m_splitterRight, rcDefault, NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		ATLASSERT(m_tabview.m_hWnd != NULL);

		m_propertyGrid.CreateDockableWindow(m_hWnd, "Properties"sv);
		ATLASSERT(m_propertyGrid.m_hWnd != NULL);

		g_pPropertyGrid = &m_propertyGrid;

		m_treeview.CreateDockableWindow(m_hWnd, "SystemTree"sv);
		ATLASSERT(m_treeview.m_hWnd != NULL);

		m_wndOutput.CreateDockableWindow(m_hWnd, "Output"sv);

		m_splitterHor.SetSplitterPanes(m_splitterLeft, tabContainerWrapperBottom);
		m_splitterLeft.SetSplitterPanes(tabContainerWrapperLeft, m_splitterRight);
		m_splitterRight.SetSplitterPanes(m_tabview, tabContainerWrapperRight);

		DockWindow(m_wndOutput, false);
		DockWindow(m_treeview, false);
		DockWindow(m_propertyGrid, false);

		UpdateLayout();
		UIAddToolBar(hWndToolBar);
		UISetCheck(ID_VIEW_TOOLBAR, 1);
		UISetCheck(ID_VIEW_STATUS_BAR, 1);
		UISetCheck(ID_VIEW_TREEPANE, 1);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		ShowRibbonUI(bRibbonUI);
		UISetCheck(ID_VIEW_RIBBON, bRibbonUI);

		/*
		// setting up colors
		ATL::CComQIPtr<IPropertyStore> ips(GetIUIFrameworkPtr());
		if (ips != NULL) {

			auto set_color = [&](REFPROPERTYKEY key, UI_HSBCOLOR color) {
				PROPVARIANT prop;
				InitPropVariantFromUInt32(color, &prop);
				ips->SetValue(key, prop);
			};

			set_color(UI_PKEY_GlobalBackgroundColor, UI_HSB(195, 204, 191));
			set_color(UI_PKEY_GlobalHighlightColor, UI_HSB(55, 240, 200));
			set_color(UI_PKEY_GlobalTextColor, UI_HSB(0, 0, 0));

			ips->Commit();
		}
		*/

		//		SetTimer(TMR_1S, 1000);

		m_tabview.SetFocus(true);

		std::cout.flush();

		return 0;
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		CDlg_DocList dlg(m_tabview);
		if (dlg.Modified() && dlg.DoModal() == IDCANCEL) return 0;
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
		bHandled = FALSE;

		WINDOWPLACEMENT info;
		info.length = sizeof(WINDOWPLACEMENT);

		GetWindowPlacement(&info);

		global.controls.fullscreen = (info.showCmd == SW_SHOWMAXIMIZED) | static_cast<bool>(info.flags & WPF_RESTORETOMAXIMIZED);
		global.controls.rc_main_wnd = info.rcNormalPosition;

		/*
		std::stringstream ss;
		ss << "spliter_pos" << m_splitter.GetSplitterPosPct()
			<< "\r\nspliter_r_pos" << m_splitter_r.GetSplitterPosPct();
		MessageBoxA(0, ss.str().c_str(), 0, 0);
		*/

		//	KillTimer(TMR_1S);

		m_treeview.SaveTree();
		global.controls.Save();
		m_tabview.RemoveAllTabs();

		return 1;
	}

	LRESULT OnPanelAttached(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		//std::cout << "Main::OnPanelAttached()" << std::endl;
		//m_hwndPanels[(size_t)lParam] = (HWND)wParam;
		return 0;
	}

	LRESULT OnTabViewItemFocusSet(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		m_tabContainerBottom.Hide();
		m_tabContainerLeft.Hide();
		m_tabContainerRight.Hide();
		return 0;
	}


	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		auto cx = LOWORD(lParam), cy = HIWORD(lParam);
		//std::cout << "Main::OnSize()" << std::endl;
		//for (auto hwnd : m_hwndPanels) {
		//	if (hwnd) ::SendMessage(hwnd, WM_SIZE, 0, lParam);
		//}

		//m_tabContainerLeft.UpdateLayoutUnpinned(cx, cy);
		//m_tabContainerBottom.UpdateLayoutUnpinned(cx, cy);
		m_tabContainerRight.UpdateLayoutUnpinned(cx, cy);

		bHandled = FALSE;

		return 0;
	}

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		// TODO: add code to initialize document
//		CAlgorithmView* wnd = new CAlgorithmView();
//		wnd->Create(m_tabview, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
//		m_tabview.AddPage(wnd->m_hWnd, "Page");
		return 0;
	}

	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		static BOOL bVisible = TRUE;	// initially visible
		bVisible = !bVisible;
		CReBarCtrl rebar = m_hWndToolBar;
		int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
		rebar.ShowBand(nBandIndex, bVisible);
		UISetCheck(ID_VIEW_TOOLBAR, bVisible);
		UpdateLayout();
		return 0;
	}

	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
		::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
		UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
		UpdateLayout();
		return 0;
	}

	LRESULT OnViewRibbon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		ShowRibbonUI(!IsRibbonUI());
		UISetCheck(ID_VIEW_RIBBON, IsRibbonUI());
		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

	//	LRESULT OnViewTreePane(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	//		bool bShow = (m_splitterLeft.GetSinglePaneMode() != SPLIT_PANE_NONE);
	//		m_splitterLeft.SetSinglePaneMode(bShow ? SPLIT_PANE_NONE : SPLIT_PANE_RIGHT);
	//		UISetCheck(ID_VIEW_TREEPANE, bShow);
	//
	//		return 0;
	//	}

	//	LRESULT OnTreePaneClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/) {
	//		if (hWndCtl == m_pane.m_hWnd) {
	//			m_splitterLeft.SetSinglePaneMode(SPLIT_PANE_RIGHT);
	//			UISetCheck(ID_VIEW_TREEPANE, 0);
	//		}
	//
	//		return 0;
	//	}

	LRESULT OnRibbonGalleryCtrl(UI_EXECUTIONVERB verb, WORD wID, UINT uSel, BOOL& bHandled) {

	}

	LRESULT OnStatusBarUpdate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		m_statusBar.Update(m_tabview.GetActiveView());
		return 0;
	}

	LRESULT OnSelectCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/) {
		g_ribbonState.cursor_selected = true;
		g_ribbonState.block_selected = -1;
		m_ribbonFBDBlocks.Select(-1, true);
		m_ribbonSFCBlocks.Select(-1, true);
		return 0;
	}

	void ToggleDockableWindow(CDockableWindow& wnd, std::string_view windowName) {
		if (wnd) {
			auto tab = GetTabContainer(wnd.GetDock());
			ATLASSERT(tab);
			tab->CloseWindow(wnd);
		} else {
			wnd.CreateDockableWindow(m_hWnd, windowName);
			DockWindow(wnd);
		}
	}

	LRESULT OnCheckViewSystemTree(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/) {
		ToggleDockableWindow(m_treeview, "SystemTree"sv);
		return 0;
	}

	LRESULT OnCheckViewOutput(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/) {
		ToggleDockableWindow(m_wndOutput, "Output"sv);
		return 0;
	}

	LRESULT OnCheckViewPropertyGrid(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/) {
		ToggleDockableWindow(m_propertyGrid, "Properties"sv);
		return 0;
	}

	void UpdateUIAll() {
		auto view = m_tabview.GetActiveView();
		if (view == nullptr) {
			UIEnable(ID_FILE_SAVE, FALSE);
			UIEnable(ID_EDIT_UNDO, FALSE);
			UIEnable(ID_EDIT_REDO, FALSE);
			UIEnable(ID_START_ONLINE, FALSE);
			UIEnable(ID_STOP_ONLINE, FALSE);
			UIEnable(ID_UPLOAD_ALGORITHM, FALSE);
		} else {
			UIEnable(ID_FILE_SAVE, view->IsModified());
			UIEnable(ID_EDIT_UNDO, view->CanUndo());
			UIEnable(ID_EDIT_REDO, view->CanRedo());
			UIEnable(ID_START_ONLINE, view->CanStartOnline());
			UIEnable(ID_STOP_ONLINE, view->CanStopOnline());
			UIEnable(ID_UPLOAD_ALGORITHM, view->CanUpload());
		}

		SetProperty(ID_GALLERY_FBD_BLOCKS, UI_PKEY_Enabled, g_ribbonState.fbd_ribbon_active ? m_propertyTrue : m_propertyFalse);
		SetProperty(ID_GALLERY_SFC_BLOCKS, UI_PKEY_Enabled, g_ribbonState.sfc_ribbon_active ? m_propertyTrue : m_propertyFalse);
		
		SetProperty(ID_SELECT_CURSOR, UI_PKEY_BooleanValue, g_ribbonState.cursor_selected ? m_propertyTrue : m_propertyFalse);
		SetProperty(ID_VIEW_SYSTEM_TREE, UI_PKEY_BooleanValue, m_treeview ? m_propertyTrue : m_propertyFalse);
		SetProperty(ID_VIEW_OUTPUT, UI_PKEY_BooleanValue, m_wndOutput ? m_propertyTrue : m_propertyFalse);
		SetProperty(ID_VIEW_PROPERTY_GRID, UI_PKEY_BooleanValue, m_propertyGrid ? m_propertyTrue : m_propertyFalse);
	}

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		/*
		try {
			auto status = npsys_rpc->server->GetNetworkStatus();
			for (size_t i = 0; i < 32; ++i) {
				std::cout << (int)status[i] << " ";
			}
			std::cout << std::endl;
			cbt::network_status_free(status);
		} catch (CORBA::Exception& ex) {
			std::cerr << ex._name() << '\n';
		}
		*/
		return 0;
	}
};