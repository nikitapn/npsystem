// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "tabcontainer.h"
#include "global.h"
#include "gdiglobal.h"

extern HWND g_hMainWndClient;

using Dock = Dock;

enum {
	splitter_width = 4,
	cxyHeader = 30
};

// CMyPaneContainer

class CMyPaneContainer
	: public CWindowImpl<CMyPaneContainer>
	, public CCustomDraw<CMyPaneContainer> {
	friend CCustomDraw<CMyPaneContainer>;

	constexpr static auto m_max_title_length = 120;
	constexpr static auto m_nButtons = 3;

	constexpr static auto m_nCollapseBtnID = ID_PANE_COLLAPSE;
	constexpr static auto m_nPinBtnID = ID_PANE_PIN;
	constexpr static auto m_nCloseBtnID = ID_PANE_CLOSE;

	constexpr static auto m_nCollapseBtnIndex = 0;
	constexpr static auto m_nPinBtnIndex = 1;
	constexpr static auto m_nCloseBtnIndex = 2;

	constexpr static auto m_cxyBtn = 10;
	constexpr static auto m_xBtnImageLeft = 6;
	constexpr static auto m_yBtnImageTop = 6;
	constexpr static auto m_xBtnImageRight = m_xBtnImageLeft + m_cxyBtn;
	constexpr static auto m_yBtnImageBottom = m_yBtnImageTop + m_cxyBtn;
	constexpr static auto m_cxyImageTB = m_xBtnImageRight;

	constexpr static auto m_cxyBtnAddTB = 7;

	constexpr static auto m_cxToolBar = (m_cxyImageTB + m_cxyBtnAddTB) * m_nButtons;
	constexpr static auto m_cyToolBar = m_cxyImageTB + m_cxyBtnAddTB;

	constexpr static auto m_cxyContentOffset = 3;
	constexpr static auto m_cxyHeader = m_cyToolBar + 2 * m_cxyContentOffset;


	size_t m_index;
	CMyTabContainerImpl& m_tabContainer;
	TCHAR m_szTitle[m_max_title_length];
	CToolBarCtrl m_tb;

public:
	CWindow m_wndClient;

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		Init();
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
		UINT cx = LOWORD(lParam), cy = HIWORD(lParam);
		UpdateLayout(cx, cy);
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		DrawPaneTitleBackground((HDC)wParam);
		return 1;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		if (wParam != NULL) {
			DrawPaneTitle((HDC)wParam);
			if (m_wndClient.m_hWnd == NULL)   // no client window
				DrawPane((HDC)wParam);
		} else {
			CPaintDC dc(this->m_hWnd);
			DrawPaneTitle(dc.m_hDC);
			if (m_wndClient.m_hWnd == NULL)   // no client window
				DrawPane(dc.m_hDC);
		}
		return 0;
	}

	void Init() {
		CreateButtons();
	}

	void CreateButtons() {
		ATLASSERT(m_tb.m_hWnd == NULL);

		m_tb.Create(this->m_hWnd, this->rcDefault, NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN
			| CCS_NOMOVEY | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT, 0);

		ATLASSERT(m_tb.IsWindow());

		if (m_tb.m_hWnd != NULL) {
			m_tb.SetButtonStructSize();

			constexpr auto nButtons = 3;

			TBBUTTON tbbtn[nButtons] = {};
			tbbtn[0].idCommand = m_nCollapseBtnID;
			tbbtn[0].fsState = TBSTATE_ENABLED;
			tbbtn[0].fsStyle = BTNS_BUTTON;
			tbbtn[0].dwData = m_nCollapseBtnIndex;

			tbbtn[1].idCommand = m_nPinBtnID;
			tbbtn[1].fsState = TBSTATE_ENABLED;
			tbbtn[1].fsStyle = BTNS_BUTTON;
			tbbtn[1].dwData = m_nPinBtnIndex;

			tbbtn[2].idCommand = m_nCloseBtnID;
			tbbtn[2].fsState = TBSTATE_ENABLED;
			tbbtn[2].fsStyle = BTNS_BUTTON;
			tbbtn[2].dwData = m_nCloseBtnIndex;

			m_tb.AddButtons(nButtons, tbbtn);

			m_tb.SetBitmapSize(m_cxyImageTB, m_cxyImageTB);
			m_tb.SetButtonSize(m_cxyImageTB + m_cxyBtnAddTB, m_cxyImageTB + m_cxyBtnAddTB);

			m_tb.SetWindowPos(NULL, 0, 0, (m_cxyImageTB + m_cxyBtnAddTB) * nButtons, m_cxyImageTB + m_cxyBtnAddTB,
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
	}

	void DrawPane(CDCHandle dc) {
		RECT rect = {};
		this->GetClientRect(&rect);
		rect.top += m_cxyHeader;
		if ((this->GetExStyle() & WS_EX_CLIENTEDGE) == 0)
			dc.DrawEdge(&rect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
		dc.FillRect(&rect, COLOR_APPWORKSPACE);
	}

	LRESULT OnNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		if (m_tb.m_hWnd == NULL) {
			bHandled = FALSE;
			return 1;
		}

		LPNMHDR lpnmh = (LPNMHDR)lParam;
		LRESULT lRet = 0;

		// pass toolbar custom draw notifications to the base class
		if ((lpnmh->code == NM_CUSTOMDRAW) && (lpnmh->hwndFrom == m_tb.m_hWnd)) {
			lRet = OnCustomDraw(0, lpnmh, bHandled);
			// tooltip notifications come with the tooltip window handle and button ID,
			// pass them to the parent if we don't handle them
		} else if ((lpnmh->code == TTN_GETDISPINFO) && (lpnmh->idFrom == m_nCloseBtnID)) {
			// bHandled = GetToolTipText(lpnmh);
		}
		// only let notifications not from the toolbar go to the parent
		else if ((lpnmh->hwndFrom != m_tb.m_hWnd) && (lpnmh->idFrom >= m_nCollapseBtnID && lpnmh->idFrom <= m_nCloseBtnID)) {
			bHandled = FALSE;
		}

		return lRet;
	}

	void DrawButton_X(CDCHandle dc, const CRect& rcImage) {
		dc.MoveTo(rcImage.left, rcImage.top);
		dc.LineTo(rcImage.right, rcImage.bottom);
		dc.MoveTo(rcImage.left + 1, rcImage.top);
		dc.LineTo(rcImage.right + 1, rcImage.bottom);

		dc.MoveTo(rcImage.left, rcImage.bottom - 1);
		dc.LineTo(rcImage.right, rcImage.top - 1);
		dc.MoveTo(rcImage.left + 1, rcImage.bottom - 1);
		dc.LineTo(rcImage.right + 1, rcImage.top - 1);
	}

	void DrawButton_Collapse(CDCHandle dc, const CRect& rcImage) {
		constexpr auto ox = 2;
		constexpr auto oy = 2;
		dc.MoveTo(rcImage.left + ox, rcImage.top + oy);
		dc.LineTo(rcImage.right - ox, rcImage.top + oy);
		dc.LineTo(rcImage.left + rcImage.Width() / 2, rcImage.bottom - oy - 2);
		dc.LineTo(rcImage.left + ox, rcImage.top + oy);
	}

	// Custom draw overrides
	DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/) {
		return CDRF_NOTIFYITEMDRAW;   // we need per-item notifications
	}

	DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/) {
		return CDRF_NOTIFYPOSTPAINT;
	}

	DWORD OnItemPostPaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw) {
		auto dc = lpNMCustomDraw->hdc;
		auto const& rc = lpNMCustomDraw->rc;

		CRect rcImage = { m_xBtnImageLeft, m_yBtnImageTop, m_xBtnImageRight, m_yBtnImageBottom };
		rcImage.OffsetRect(rc.left, rc.top);

		CPen pen1;
		pen1.CreatePen(PS_SOLID, 0, ::GetSysColor(COLOR_3DHILIGHT));

		switch (lpNMCustomDraw->lItemlParam) {
		case m_nCloseBtnIndex:
			DrawButton_X(dc, rcImage);
			break;
		case m_nPinBtnIndex:
			DrawButton_Pin(dc, rcImage);
			break;
		case m_nCollapseBtnIndex:
			DrawButton_Collapse(dc, rcImage);
			break;
		}

		return CDRF_DODEFAULT;   // continue with the default item painting
	}
	void DrawButton_Pin(CDCHandle dc, const CRect& rcImage);
	void DrawPaneTitle(CDCHandle dc);
	void UpdateLayout(int cxWidth, int cyHeight);
	void DrawPaneTitleBackground(CDCHandle dc);
public:
	DECLARE_WND_CLASS_EX(L"CMyPaneContainer", 0, -1)

	BEGIN_MSG_MAP(CMyPaneContainer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	CMyPaneContainer(CMyTabContainerImpl& tab_container, size_t index)
		: m_tabContainer(tab_container)
		, m_index(index) {
		m_szTitle[0] = L'\0';
	}

	HWND Create(HWND hWndParent, LPCTSTR lpstrTitle = NULL, 
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		DWORD dwExStyle = 0, UINT nID = 0, LPVOID lpCreateParam = NULL) {
		if (lpstrTitle != NULL) {
			ATL::Checked::tcsncpy_s(m_szTitle, m_max_title_length, lpstrTitle, _TRUNCATE);
		}
		return __super::Create(hWndParent, this->rcDefault, NULL, dwStyle, dwExStyle, nID, lpCreateParam);
	}

	HWND SetClient(CDockableWindow& hWndClient) {
		HWND hWndOldClient = m_wndClient;
		m_wndClient = hWndClient.m_hWnd;
		if (this->m_hWnd != NULL) {
			CRect rcWnd;
			GetClientRect(&rcWnd);
			UpdateLayout(rcWnd.Width(), rcWnd.Height());
		}
		return hWndOldClient;
	}

	void SetIndex(size_t index) noexcept {
		m_index = index;
	}

	BOOL ShowWindow(_In_ int nCmdShow) throw() {
		ATLASSERT(::IsWindow(m_hWnd));
		if (m_wndClient) m_wndClient.ShowWindow(nCmdShow);
		return ::ShowWindow(m_hWnd, nCmdShow);
	}

	void DestroyPane() {
		if (m_wndClient) {
			m_wndClient.DestroyWindow();
			m_wndClient = NULL;
		}
		DestroyWindow();
	}
	//
	virtual void OnFinalMessage(_In_ HWND /*hWnd*/);
}; // class CMyPaneContainer

// class CSplitterChildWnd

class CMyTabContainerImpl;
class CMyTabHeader;

class CMySplitterChildWnd : public CWindowImpl<CMySplitterChildWnd> {
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
public:
	std::unique_ptr<CMyTabHeader> m_header;
	CMyTabContainerImpl& m_tabContainer;

	DECLARE_WND_CLASS(L"CSplitterChildWnd")

	CMySplitterChildWnd(CMyTabContainerImpl& tabContainer);

	BEGIN_MSG_MAP(CMySplitterChildWnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()
};

//  CMyTabContainerImpl

class CMyTabContainerImpl : public CWindowImpl<CMyTabContainerImpl> {
	friend class CMyTabHeader;

	CMySplitterChildWnd m_wndWrapper;
	CMySplitter* m_splitter = nullptr;

	struct Tab {
		CRect rc;
		CRect rcPosition;
		std::wstring title;
		std::unique_ptr<CMyPaneContainer> pane;
		CDockableWindow* wnd;

		Tab(CMyTabContainerImpl& tabContainer, CDockableWindow& _wnd, const std::string& _title, size_t index)
			: wnd(&_wnd) 
		{
			title = wide(_title);

			CClientDC dc(tabContainer);
			dc.SelectFont(tabContainer.GetDock() == Dock::Bottom ? gdiGlobal.fntTreeView : gdiGlobal.fntVerticalLeft);
			dc.DrawText(title.c_str(), static_cast<int>(title.length()), &rc, DT_CALCRECT);

			
			pane.reset(new CMyPaneContainer(tabContainer, index));
			pane->Create(tabContainer, title.c_str(), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			//pane->SetPaneContainerExtendedStyle(PANECNT_NOBORDER);
			
			wnd->SetParent(tabContainer);
			pane->SetClient(*wnd);
		}

		bool operator==(const CDockableWindow& _wnd) const noexcept {
			return pane->m_wndClient == _wnd.m_hWnd;
		}
	};

	size_t m_activeTab = -1;
	size_t m_dirty_tabs_from = 0;
	std::vector<Tab> m_tabs;
	const Dock m_dock;
	CMyTabContainer::StoredState m_state;
	int m_mainClientWndCX;
	int m_mainClientWndCY;
public:
	using base = CWindowImpl<CMyTabContainerImpl>;

	DECLARE_WND_CLASS(L"CMyTabContainer")

	BEGIN_MSG_MAP(CMyTabContainer)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_NCCALCSIZE, OnNCCalcSize)
		MESSAGE_HANDLER(WM_NCHITTEST, OnNCHitTest)
		COMMAND_ID_HANDLER(ID_PANE_CLOSE, OnTreePaneClose)
		COMMAND_ID_HANDLER(ID_PANE_PIN, OnTreePanePin)
		COMMAND_ID_HANDLER(ID_PANE_COLLAPSE, OnTreePaneCollapse)
	END_MSG_MAP()

	Dock GetDock() const noexcept {
		return m_dock;
	}

	auto IsPinned() const noexcept {
		return m_state.pinned;
	}

	auto IsVertical() const noexcept {
		m_splitter->m_bVertical;
	}

	auto Empty() const noexcept {
		return m_tabs.empty();
	}

	const CMyTabContainer::StoredState& GetStoredState() noexcept{
		m_state.splitter_cxy_pos = m_splitter->GetSplitterRelativePos();
		return m_state;
	}

	CMyTabContainerImpl(CMyTabContainer* top, Dock dock)
		: m_dock(dock)
		, m_wndWrapper(*this)
		, m_state(CMyTabContainer::states[static_cast<size_t>(dock)]) 
	{
		CMyTabContainer::states_ptr[static_cast<size_t>(dock)] = top;
	}

	HWND Create(CMySplitter& splitter) {
		m_splitter = &splitter;

		auto hWnd = m_wndWrapper.Create(splitter, rcDefault,
			NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		if (IsPinned() == false) {
			m_splitter->SetSplitterRelativePos(cxyHeader);
			m_splitter->SetSplitterExtendedStyle(SPLIT_NONINTERACTIVE, SPLIT_NONINTERACTIVE);
		}

		return hWnd;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		UINT cx = LOWORD(lParam), cy = HIWORD(lParam);
		UpdateLayout(cx, cy);
		return 0;
	}

	void UpdateLayout(int cx = -1, int cy = -1) {
		if (cx == -1) {
			CRect rc;
			GetClientRect(&rc);
			cx = rc.Width(); cy = rc.Height();
		}

		if (m_activeTab != -1) {
			auto& tab = m_tabs[m_activeTab];
			auto pane = m_tabs[m_activeTab].pane.get();

			auto offset = IsPinned() ? 0 : splitter_width;

			if (GetDock() == Dock::Bottom) {
				pane->SetWindowPos(NULL, 0, offset, cx, cy - offset, SWP_NOZORDER);
			} else if (GetDock() == Dock::Left) {
				pane->SetWindowPos(NULL, 0, 0, cx - offset, cy, SWP_NOZORDER);
			} else {
				pane->SetWindowPos(NULL, offset, 0, cx - offset, cy, SWP_NOZORDER);
			}
		}
	}


	void GetUnpinnedRect(const Tab& tab, CRect& rcWnd) {
		if (tab.wnd->m_state.unpinned_cxy < 10) tab.wnd->m_state.unpinned_cxy = 10;

		if (m_dock == Dock::Bottom) {
				rcWnd.left = 0;
				rcWnd.top = m_mainClientWndCY - cxyHeader - tab.wnd->m_state.unpinned_cxy;
				rcWnd.right = m_mainClientWndCX;
				rcWnd.bottom = m_mainClientWndCY - cxyHeader;
			} else if (m_dock == Dock::Left) {
				rcWnd.left = cxyHeader;
				rcWnd.top = 0;
				rcWnd.right = cxyHeader + tab.wnd->m_state.unpinned_cxy;
				rcWnd.bottom = m_mainClientWndCY - cxyHeader;
			} else {
				rcWnd.left = m_mainClientWndCX - cxyHeader - tab.wnd->m_state.unpinned_cxy;
				rcWnd.top = 0;
				rcWnd.right = m_mainClientWndCX - cxyHeader;
				rcWnd.bottom = m_mainClientWndCY - cxyHeader;
			}
	}

	void UpdateLayoutUnpinned(int cx, int cy) {
		m_mainClientWndCX = cx; m_mainClientWndCY = cy;
		if (IsPinned() == true) return;

		if (m_activeTab != -1) {
			ATLASSERT(GetParent() == g_hMainWndClient);
			auto& tab = m_tabs[m_activeTab];
			CRect rcWnd;
			GetUnpinnedRect(tab, rcWnd);
			SetWindowPos(NULL, &rcWnd, SWP_SHOWWINDOW);
		}
	}

	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		CDCHandle dc((HDC)wParam);

		if (!IsPinned()) {
			CRect rc;
			GetClientRect(&rc);
			auto dock = GetDock();
			if (dock == Dock::Bottom) {
				rc.bottom = rc.top + splitter_width;
			} else if (dock == Dock::Left) {
				rc.left = rc.right - splitter_width;
			} else {
				rc.right = rc.left + splitter_width;
			}
			
			dc.FillSolidRect(rc, gdiclr::gray);
		}

		return 0;
	}

	LRESULT OnTreePaneClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/) {
		m_tabs[m_activeTab].pane->DestroyPane();
		return 0;
	}

	void CloseWindow(CDockableWindow& wnd) {
		auto founded = std::find(m_tabs.begin(), m_tabs.end(), wnd);
		ATLASSERT(founded != m_tabs.end());
		auto ix = std::distance(m_tabs.begin(), founded);
		founded->pane->DestroyPane();
	}

	LRESULT OnTreePanePin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/) {
		m_state.pinned ^= true;

		if (IsPinned() == false) { // unpin window
			SetFocus();

			m_splitter->SetSplitterExtendedStyle(SPLIT_NONINTERACTIVE, SPLIT_NONINTERACTIVE);
			m_splitter->SetSplitterRelativePos(cxyHeader);
			
			SetParent(g_hMainWndClient);

			CRect rcWnd;
			auto& tab = m_tabs[m_activeTab];
			GetUnpinnedRect(tab, rcWnd);
			SetWindowPos(NULL, rcWnd, SWP_SHOWWINDOW);
		} else {
			CRect rcWnd;
			GetClientRect(&rcWnd);

			SetParent(m_wndWrapper);

			m_splitter->SetSplitterExtendedStyle(0, SPLIT_NONINTERACTIVE);
			m_splitter->SetSplitterRelativePos(
				(m_dock == Dock::Bottom ? rcWnd.Height() : rcWnd.Width()) + cxyHeader + 1);
		}

		return 0;
	}

	LRESULT OnTreePaneCollapse(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/) {
		if (IsPinned()) {
			m_splitter->SetSplitterRelativePos(cxyHeader);
		} else {
			SelectWindow(-1);
		}
		return 0;
	}

	LRESULT OnNCCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		if (wParam == TRUE) {
			//auto ncp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
		}
		return 0;
	}

	LRESULT OnNCActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return 0;
	}

	LRESULT OnNCHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		// identify borders and corners to allow resizing the window.
		// Note: On Windows 10, windows behave differently and
		// allow resizing outside the visible window frame.
		// This implementation does not replicate that behavior.
		CPoint cursor(lParam);

		const POINT border{
				::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER),
				::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER)
		};

		RECT window;
		if (!::GetWindowRect(m_hWnd, &window)) {
			return HTNOWHERE;
		}

		enum region_mask {
			client = 0b0000,
			left = 0b0001,
			right = 0b0010,
			top = 0b0100,
			bottom = 0b1000,
		};

		const auto result =
			left * (cursor.x < (window.left + border.x)) |
			right * (cursor.x >= (window.right - border.x)) |
			top * (cursor.y < (window.top + border.y)) |
			bottom * (cursor.y >= (window.bottom - border.y));

		if (IsPinned() && result != client) return HTNOWHERE;

		switch (result) {
		case left: return HTLEFT;
		case right: return HTRIGHT;
		case top: return HTTOP;
			//case bottom: return HTBOTTOM;
			//case top | left: return HTTOPLEFT;
			//case top | right: return HTTOPRIGHT;
			//case bottom | left: return HTBOTTOMLEFT;
			//case bottom | right: return HTBOTTOMRIGHT;
		case client: return HTCLIENT;
		default: return HTNOWHERE;
		}
		return 0;
	}

	void DockWindow(CDockableWindow& wnd, bool show_window) {
		assert(GetDock() == wnd.GetDock());
		auto index = m_tabs.size();
		m_tabs.emplace_back(*this, wnd, std::string{ wnd.GetWindowTitle() }, index);

		if (index == 0) {
			m_splitter->SetSplitterMinPosition(cxyHeader);
			if (IsPinned()) {
				m_splitter->SetSplitterRelativePos(m_state.splitter_cxy_pos);
			} else {
				m_splitter->SetSplitterRelativePos(cxyHeader);
				//m_splitter->SetSplitterExtendedStyle(SPLIT_NONINTERACTIVE);
			}
		}
		if (IsPinned() || show_window) SelectWindow(index);
		m_dirty_tabs_from = index;
	}

	void StoreUnpinnedCxCy(size_t index) {
		ATLASSERT(index < m_tabs.size());
		ATLASSERT(IsPinned() == false);

		auto& tab = m_tabs[index];
		CRect rcWnd;
		GetClientRect(&rcWnd);
		
		if (m_dock == Dock::Bottom) {
			tab.wnd->m_state.unpinned_cxy = rcWnd.Height();
		} else {
			tab.wnd->m_state.unpinned_cxy = rcWnd.Width();
		}
	}

	void SelectWindow(size_t index) {
		ATLASSERT(m_tabs.size() > 0);

		if (m_activeTab == index) return;

		if (index == -1 && IsPinned() == false) {
			StoreUnpinnedCxCy(m_activeTab);
			ShowWindow(SW_HIDE);
			m_activeTab = -1;
			return;
		}  

		if (m_activeTab == -1 && index != -1) {
			m_tabs[index].pane->ShowWindow(SW_SHOW);
		} else if (index != -1 && m_activeTab != -1){
			StoreUnpinnedCxCy(m_activeTab);
			m_tabs[m_activeTab].pane->ShowWindow(SW_HIDE);
			m_tabs[index].pane->ShowWindow(SW_SHOW);
		}

		m_activeTab = index;

		SetFocus();

		if (IsPinned() == false) {
			auto& tab = m_tabs[m_activeTab];
			CRect rcWnd;
			GetUnpinnedRect(tab, rcWnd);
			SetWindowPos(NULL, rcWnd, SWP_SHOWWINDOW);
		}

		UpdateLayout();
		
		//auto dy = IsPinned() ? 0 : splitter_width;
		//tab.pane->SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOZORDER);
		/*
		auto dock = GetDock();
		auto offset = IsPinned() ? 0 : splitter_width;
		if (dock == Dock::Bottom) {
			tab.pane->SetWindowPos(NULL, 0, offset, cx, cy - offset, SWP_NOZORDER);
		} else if (dock == Dock::Left) {
			tab.pane->SetWindowPos(NULL, 0, 0, cx - offset, cy, SWP_NOZORDER);
		} else {
			tab.pane->SetWindowPos(NULL, offset, 0, cx - offset, cy, SWP_NOZORDER);
		}
		*/
	}

	void OnItemDestroyed(size_t itemIndex);
};

// class CMyTabHeader

class CMyTabHeader : public CWindowImpl<CMyTabHeader> {
public:
	enum {
		cyOffsetText = 5,
		cyText = cxyHeader - cyOffsetText * 2,
		cxBetweenItem = 5,
	};
private:
	CMyTabContainerImpl& m_tabContainer;

	bool m_mouse_tracked = false;
	size_t m_idx_hovered = -1;

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		if (!m_tabContainer.m_tabs.empty()) m_tabContainer.m_dirty_tabs_from = 0;
		return 0;
	}

	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		CDCHandle dc((HDC)wParam);

		CRect rc;
		GetClientRect(&rc);
		dc.FillSolidRect(&rc, RGB(222, 222, 222));
		
		return 0;
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		CPaintDC dc(m_hWnd);

		CRect rc;
		GetClientRect(&rc);

		auto dock = m_tabContainer.GetDock();

		if (m_tabContainer.m_dirty_tabs_from != -1) {
			UpdateTabsPosition(m_tabContainer.m_dirty_tabs_from, rc, dock);
			m_tabContainer.m_dirty_tabs_from = -1;
		}

		COLORREF color = gdiGlobal.clrStatusBar;

		dc.SetBkColor(RGB(222, 222, 222));
		dc.SetTextColor(gdiclr::black);
		dc.SelectFont(dock == Dock::Bottom ? gdiGlobal.fntTreeView :
			dock == Dock::Right ? gdiGlobal.fntVerticalRight : gdiGlobal.fntVerticalLeft);

		for (size_t i = 0; i < m_tabContainer.m_tabs.size(); ++i) {
			auto& tab = m_tabContainer.m_tabs[i];
			if (i == m_idx_hovered) {
				color = gdiclr::blue;
				dc.SetTextColor(gdiclr::blue);
			}

			CRect rc = tab.rcPosition;

			if (dock == Dock::Bottom) {
				rc.top += cyOffsetText;
				rc.bottom -= cyOffsetText;
				dc.DrawText(tab.title.c_str(), static_cast<int>(tab.title.length()), &rc,
					DT_CENTER | DT_VCENTER);
			} else {
				rc.left += cyOffsetText;
				rc.right -= cyOffsetText;

				if (dock == Dock::Right) {
					dc.TextOutW(rc.right, rc.top, tab.title.c_str(), static_cast<int>(tab.title.length()));
				} else {
					dc.TextOutW(rc.left, rc.bottom, tab.title.c_str(), static_cast<int>(tab.title.length()));
				}
			}

			if (dock == Dock::Bottom) {
				rc.top = rc.bottom;
				rc.bottom += cyOffsetText;
			} else if (dock == Dock::Right) {
				rc.left = rc.right;
				rc.right += cyOffsetText;
			} else {
				rc.left = 0;
				rc.right = cyOffsetText;
			}

			dc.FillSolidRect(rc, color);

			if (i == m_idx_hovered) {
				color = gdiGlobal.clrStatusBar;
				dc.SetTextColor(gdiclr::black);
			}
		}

		return 0;
	}

	void UpdateTabsPosition(size_t fromIdx, const CRect& rcClient, Dock dock) {
		auto& tabs = m_tabContainer.m_tabs;

		if (tabs.size() == 0) return;

		ATLASSERT(fromIdx < tabs.size());
		if (dock == Dock::Bottom) {
			int x = 0;

			if (fromIdx == 0) x = cyOffsetText;
			else x = tabs[fromIdx - 1].rcPosition.right + cyOffsetText;

			for (size_t i = fromIdx; i < tabs.size(); ++i) {
				tabs[i].rcPosition.left = x;
				tabs[i].rcPosition.top = rcClient.top;
				tabs[i].rcPosition.right = x + tabs[i].rc.right;
				tabs[i].rcPosition.bottom = rcClient.bottom;
				x = tabs[i].rcPosition.right + cyOffsetText;
			}
		} else {
			int y = 0;

			if (fromIdx == 0) y = cyOffsetText;
			else y = tabs[fromIdx - 1].rcPosition.bottom + cyOffsetText;

			for (size_t i = fromIdx; i < tabs.size(); ++i) {
				tabs[i].rcPosition.left = rcClient.left;
				tabs[i].rcPosition.top = y;
				tabs[i].rcPosition.right = rcClient.right;
				tabs[i].rcPosition.bottom = y + tabs[i].rc.right;

				y = tabs[i].rcPosition.bottom + cyOffsetText;
			}
		}
	}

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		CPoint pt(lParam);

		auto tab_index = GetTabIndexFromPt(pt);

		if (tab_index != -1) {
			if (m_idx_hovered != tab_index) {
				m_idx_hovered = tab_index;
				TrackMouse();
				Invalidate();
			}
			return 0;
		}

		if (m_idx_hovered != -1) {
			m_idx_hovered = -1;
			TrackMouse();
			Invalidate();
			return 0;
		}

		return 0;
	}

	void TrackMouse() {
		if (m_mouse_tracked) return;
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = 1;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
		m_mouse_tracked = true;
	}

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		CPoint pt(lParam);
		auto tab_index = GetTabIndexFromPt(pt);
		if (tab_index != -1) m_tabContainer.SelectWindow(tab_index);
		return 0;
	}

	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		m_mouse_tracked = false;
		if (m_idx_hovered != -1) {
			m_idx_hovered = -1;
			Invalidate();
			return 0;
		}
		return 0;
	}

	size_t GetTabIndexFromPt(const CPoint& pt) {
		auto begin = m_tabContainer.m_tabs.begin();
		for (auto it = begin; it != m_tabContainer.m_tabs.end(); ++it) {
			if (PtInRect(it->rcPosition, pt) == TRUE) return std::distance(begin, it);
		}
		return -1;
	}
public:
	DECLARE_WND_CLASS(L"CMyTabHeader")

	BEGIN_MSG_MAP(CMyTabHeader)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	END_MSG_MAP()

	CMyTabHeader(CMyTabContainerImpl& tabContainer)
		: m_tabContainer(tabContainer) {}
}; // class CMyTabHeader

// CMySplitterChildWnd

CMySplitterChildWnd::CMySplitterChildWnd(CMyTabContainerImpl& tabContainer)
	: m_tabContainer(tabContainer) {
	m_header = std::make_unique<CMyTabHeader>(m_tabContainer);
}

LRESULT CMySplitterChildWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	m_tabContainer.base::Create(m_tabContainer.IsPinned() ? m_hWnd : g_hMainWndClient, rcDefault,
		NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_header->Create(m_hWnd, rcDefault,
		NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	return 0;
}

LRESULT CMySplitterChildWnd::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	UINT cx = LOWORD(lParam), cy = HIWORD(lParam);
	if (m_tabContainer.IsPinned()) {
		if (m_tabContainer.GetDock() == Dock::Bottom) {
			m_tabContainer.SetWindowPos(NULL, 0, 0, cx, cy - cxyHeader, SWP_SHOWWINDOW);
			m_header->SetWindowPos(NULL, 0, cy - cxyHeader, cx, cxyHeader, SWP_SHOWWINDOW);
		} else if (m_tabContainer.GetDock() == Dock::Right) {
			m_tabContainer.SetWindowPos(NULL, 0, 0, cx - cxyHeader, cy, SWP_SHOWWINDOW);
			m_header->SetWindowPos(NULL, cx - cxyHeader, 0, cxyHeader, cy, SWP_SHOWWINDOW);
		} else {
			m_tabContainer.SetWindowPos(NULL, cxyHeader, 0, cx - cxyHeader, cy, SWP_SHOWWINDOW);
			m_header->SetWindowPos(NULL, 0, 0, cxyHeader, cy, SWP_SHOWWINDOW);
		}
	} else {
		m_header->SetWindowPos(NULL, 0, 0, cx, cy, SWP_SHOWWINDOW);
	}
	return 0;
}

// class CMyTabContainerImpl

void CMyTabContainerImpl::OnItemDestroyed(size_t itemIndex) {
	// by now the client window is hidden
	// pane window is destroyed
	size_t next_index;
	
	if (m_tabs.size() == 1) {
		next_index = -1;
	} else {
		next_index = itemIndex ? itemIndex - 1 : 0;
	}

	m_dirty_tabs_from = next_index + 1;

	for (size_t ix = itemIndex + 1; ix < m_tabs.size(); ++ix) {
		m_tabs[ix].pane->SetIndex(ix - 1);
	}

	auto it = m_tabs.begin();
	std::advance(it, itemIndex);
	m_tabs.erase(it);

	m_activeTab = -1;

	if (next_index == -1) {
		m_splitter->SetSplitterMinPosition(0);
		m_splitter->SetSplitterRelativePos(0);
		ShowWindow(SW_HIDE);
	} else {
		SelectWindow(next_index);
	}

	m_wndWrapper.m_header->Invalidate();
}

// class CMyPaneContainer

void CMyPaneContainer::OnFinalMessage(_In_ HWND /*hWnd*/) {
	if (m_wndClient) m_wndClient.ShowWindow(SW_HIDE);
	m_tabContainer.OnItemDestroyed(m_index);
}

void CMyPaneContainer::DrawButton_Pin(CDCHandle dc, const CRect& rcImage) {
	if (m_tabContainer.IsPinned()) {
		DrawIconEx(dc, rcImage.left, rcImage.top, global.GetIcon(AlphaIcon::Pinned), 9, 9, 0, NULL, DI_IMAGE | DI_MASK);
	} else {
		DrawIconEx(dc, rcImage.left, rcImage.top, global.GetIcon(AlphaIcon::UnPinned),
			rcImage.Width(), rcImage.Height(), 0, NULL, DI_IMAGE | DI_MASK);
	}
}

void CMyPaneContainer::UpdateLayout(int cxWidth, int cyHeight) {
	ATLASSERT(::IsWindow(this->m_hWnd));
	RECT rect = {};

	::SetRect(&rect, 0, 0, cxWidth, m_cxyHeader);
	if (m_tb.m_hWnd != NULL) {
		m_tb.SetWindowPos(NULL, rect.right - m_cxToolBar - m_cxyContentOffset, m_cxyContentOffset, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	if (m_wndClient.m_hWnd != NULL) {
		auto dock = m_tabContainer.GetDock();
		auto offset = m_tabContainer.IsPinned() ? 0 : splitter_width;
		if (dock == Dock::Bottom) {
			m_wndClient.SetWindowPos(NULL, 0, m_cxyHeader + offset, cxWidth, cyHeight - m_cxyHeader, SWP_NOZORDER);
		} else if (dock == Dock::Left) {
			m_wndClient.SetWindowPos(NULL, 0, m_cxyHeader, cxWidth, cyHeight - m_cxyHeader, SWP_NOZORDER);
		} else {
			m_wndClient.SetWindowPos(NULL, offset, m_cxyHeader, cxWidth, cyHeight - m_cxyHeader, SWP_NOZORDER);
		}
	} else {
		rect.bottom = cyHeight;
	}

	InvalidateRect(&rect);
}

void CMyPaneContainer::DrawPaneTitle(CDCHandle dc) {
	RECT rect = {};
	this->GetClientRect(&rect);

	UINT uBorder = BF_LEFT | BF_TOP | BF_ADJUST;
	rect.bottom = rect.top + m_cxyHeader;
	uBorder |= BF_RIGHT;

	// draw title text
	dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
	dc.SetBkMode(TRANSPARENT);

	HFONT hFontOld = dc.SelectFont(gdiGlobal.fntTreeView);

	rect.left += m_cxyContentOffset;
	rect.right -= m_cxyContentOffset;

	dc.DrawText(m_szTitle, -1, &rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

	dc.SelectFont(hFontOld);
}

void CMyPaneContainer::DrawPaneTitleBackground(CDCHandle dc) {
	RECT rect = {};
	this->GetClientRect(&rect);
	rect.bottom = m_cxyHeader;
	dc.FillRect(&rect, gdiGlobal.brFocusedTab);
}

// class CMyTabContainer

CMyTabContainer::CMyTabContainer(Dock dock) {
	impl_.reset(new CMyTabContainerImpl(this, dock));
}

HWND CMyTabContainer::Create(CMySplitter& splitter) {
	return impl_->Create(splitter);
}

void CMyTabContainer::DockWindow(CDockableWindow& wnd, bool show_window) {
	ATLASSERT(wnd.IsWindow());
	impl_->DockWindow(wnd, show_window);
}

void CMyTabContainer::ActivateWindow(size_t index) {
	impl_->SelectWindow(index);
}

void CMyTabContainer::CloseWindow(CDockableWindow& wnd) {
	impl_->CloseWindow(wnd);
}

void CMyTabContainer::Hide() {
	if (!impl_->IsPinned() && !impl_->Empty()) impl_->SelectWindow(-1);
}

__declspec(noinline) void CMyTabContainer::deleter::operator()(CMyTabContainerImpl* ptr) {
	delete ptr;
}

bool CMyTabContainer::IsPinned() const noexcept{
	return impl_->IsPinned();
}

const CMyTabContainer::StoredState& CMyTabContainer::GetStoredState() const noexcept {
	return impl_->GetStoredState();
}

void CMyTabContainer::UpdateLayoutUnpinned(int cx, int cy) {
	 impl_->UpdateLayoutUnpinned(cx, cy);
}

CMyTabContainer::operator HWND() const noexcept {
	return impl_->m_hWnd;
}
