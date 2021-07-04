// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "View.h"
#include "global.h"
#include <npdb/db.h>
#include "wm_user.h"

class CMyTabView : public CWindowImpl<CMyTabView>
{
	static constexpr LONG c_separator_height = 3;
	// distance beetween tab border and tab content
	static constexpr LONG c_dist_v = 2;
	static constexpr LONG c_dist_h = 2;
	// distance beetween text and button
	static constexpr LONG c_dist_text_btn = 4;
public:
	CMyTabView();

	DECLARE_WND_CLASS(L"CMyTabView")
	
	BEGIN_MSG_MAP(CMyTabView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	END_MSG_MAP()
protected:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	CView* GetActiveView() {
		if (m_activeTab == m_tabs.end())
			return nullptr;
		return m_activeTab->view;
	}
	// Message filter function - to be called from PreTranslateMessage of the main window
	BOOL PreTranslateMessage(MSG* pMsg) {
		if (m_activeTab == m_tabs.end())
			return 0;

		if (pMsg->message == WM_COMMAND) {
			WORD wID = LOWORD(pMsg->wParam);
			if (wID == ID_FILE_SAVE) {
				auto view = GetActiveView();
				view->Save();
				return TRUE;
			} else if ((wID > ID_RIBBON_COMMANDS_BEGIN && wID < ID_RIBBON_COMMANDS_END) ||
				(wID > ID_GALLERY_BLOCKS && wID < ID_CREATE_BLOCK_LAST)) { // all my ribbon commands
				auto view = GetActiveView();
				view->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
				return TRUE;
			} else if (wID >= 0xE123 && wID <= 0xE12C) { // default edit messages are forwarded to view
				auto view = GetActiveView();
				view->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
				return TRUE;
			}
		}
		return FALSE;
	}
	void AddView(CView* view, const std::string& title);
	void AddView(CItemView* view);
	void UpdateTitle(CView* view, const std::string& title);
	void UpdateTitle(CView* view, const std::wstring& title);
	void ActivateTab(CView* view);
	void RemoveTab(CView* view);
	void RemoveAllTabs();
	size_t GetCount() const noexcept { return m_tabs.size(); }
	auto begin() { return m_tabs.begin(); }
	auto end() { return m_tabs.end(); }
	void SetFocus(bool focus) noexcept { 
		focus_ = focus;
		InvalidateRect(m_rcTab, FALSE);
	}
	BOOL IsCursorInTab() const noexcept {
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		return m_rcWnd.PtInRect(pt);
	}

	size_t GetTabIndex(CView* view) noexcept;

	struct TabInfo {
		CView* view;
		const std::string* name;
		TreeItemState state;

		TabInfo() = default;
		TabInfo(CView* _view, const std::string* _name, TreeItemState _state) 
			: view(_view), name(_name), state(_state) {}
	};

	using SortedTabContaner = std::vector<TabInfo>;

	void InsertSorted(SortedTabContaner& items) noexcept;
	struct TBDATA
	{
		TBDATA() {
			state.size_changed = true;
			state.active = false;
		}
		TBDATA(CView* view, const std::wstring& text, LONG text_width) {
			this->view = view;
			this->text = text;
			this->text_width = text_width;
			this->state.pinned = false;
			this->state.size_changed = true;
			this->state.active = false;
		}
		CView* view;
		std::wstring text;
		std::wstring text_context;
		LONG text_width;
		CRect rcTab;
		CRect rcFreeSpace;
		CRect rcBtnPin;
		CRect rcBtnClose;
		struct State {
			bool pinned : 1;
			bool size_changed : 1;
			bool active : 1;
		} state;
	};

	TBDATA::State GetItemState(CView* view) noexcept {
		iterator it = GetIteratorFromPtr(view);
		ASSERT(it != m_tabs.end());
		return it->state;
	}
private:
	using alloc = boost::fast_pool_allocator<TBDATA,
		boost::default_user_allocator_new_delete,
		boost::details::pool::null_mutex>;

	using Tabs = std::list<TBDATA/*, alloc*/>;
	using iterator = Tabs::iterator;


	LONG GetTextWidth(const std::wstring& text);
	void ReCalcContentHeight();
	void ReCalcLayout();
	iterator GetTab(CPoint pt);
	iterator GetTab(iterator begin, iterator end, CPoint pt);
	void ActivateTab(iterator it);
	void RemoveTab(iterator it);
	iterator GetIteratorFromPtr(const CView* view) noexcept;
	void TrackMouse();
	void DeleteView(CView* view);
	void DrawCloseButton(CDCHandle dc, const CRect& rc);
	void DrawPinButton(CDCHandle dc, const CRect& rc, bool pinned);
	void Drag(const CPoint& point);
	void InvalidateTab() {
		InvalidateRect(m_rcTab, FALSE);
	}
	void PinTab(iterator ix, bool pin);
	
	
	CRect m_rcWnd;
	CRect m_rcView;
	CRect m_rcTab;
	CRect m_rcSeparator;

	LONG m_contentHeight;
	
	iterator m_pinnedBegin;
	size_t m_lastIndexPlacedInTab;
	size_t m_Lenght;
	
	iterator m_activeTab;
	iterator m_focusedTab;
	
	Tabs m_tabs;
	
	bool m_tracked;
	bool m_btnPinFocused;
	bool m_btnCloseFocused;
	bool focus_ = false;
	bool m_drag = false;

	enum class Dragged {
		Left, Right, None
	};

	Dragged m_dragged = Dragged::None;
	CPoint m_pivot;
};

#include "PropertyGrid.h"

template<typename... Args>
struct IsDialog : std::false_type {};
template<typename... Args>
struct IsDialog<CDialogImpl<Args...>> : std::true_type {};

template<typename base>
class CViewWindowT : public base 
{
public:
	CViewWindowT(CMyTabView& tabview)
		: m_tabview(tabview)
		, m_bFocused(true) 
	{
	}
	static constexpr auto is_dialog = IsDialog<base>::value;
protected:
	BEGIN_MSG_MAP(CViewWindowT)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	END_MSG_MAP()

	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		HWND hWnd = (HWND)wParam;
		if ((m_tabview != hWnd) && !m_tabview.IsCursorInTab()) {
			m_tabview.SetFocus(false);
			m_bFocused = false;
		}
		return 0;
	}
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		m_tabview.SetFocus(true);
		m_bFocused = true;
		g_pPropertyGrid->SetOwner(base::m_hWnd);
		::SendMessage(g_hMainWnd, WM_TABVIEW_FOCUS, 0, 0);
		return 0;
	}
	virtual bool IsFocused() const noexcept final {
		return m_bFocused;
	}
	CMyTabView& m_tabview;
private:
	bool m_bFocused;
};

template<typename T>
class CViewItemWindowImpl : public CViewWindowT<CWindowImpl<T, CItemView>>
{
	using base = CViewWindowT<CWindowImpl<T, CItemView>>;
public:
	CViewItemWindowImpl(CTreeItemAbstract* item, CMyTabView& tabview)
		: base(tabview) {
		base::item_ = item;
	}
};

template<typename T>
class CViewItemDialogImpl : public CViewWindowT<CDialogImpl<T, CItemView>>
{
	using base = CViewWindowT<CDialogImpl<T, CItemView>>;
public:
	CViewItemDialogImpl(CTreeItemAbstract* item, CMyTabView& tabview)
		: base(tabview) {
		base::item_ = item;
	}
};