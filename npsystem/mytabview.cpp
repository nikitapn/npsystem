// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "GDIGlobal.h"
#include "tr_item.h"
#include "MyTabView.h"
#include "dlgdoclist.h"

// distance beetween tab border and tab content
static constexpr LONG c_dist_v = 2;
static constexpr LONG c_dist_h = 2;
//
static constexpr LONG c_tab_icon_size = 32;
static constexpr LONG c_tab_height = c_tab_icon_size + c_dist_h * 2;
static constexpr LONG c_separator_height = 3;
// distance beetween text and button
static constexpr LONG c_dist_text_btn = 5;
// distance beetween buttons
static constexpr LONG c_dist_btns = 1;
// reserved space width
static constexpr LONG c_dist_reserved = 5;

constexpr auto tab_text_offset() {
	return  c_dist_v + c_tab_icon_size + c_dist_text_btn;
}

constexpr auto tab_static_width() {
	return  tab_text_offset() + /* + dynamic text size + */ c_dist_text_btn + c_dist_btns
		+ c_dist_reserved + c_dist_text_btn + c_tab_height + c_dist_btns + c_tab_height + c_dist_v;
}

CMyTabView::CMyTabView() {
	m_lastIndexPlacedInTab = 0;
	m_Lenght = 0;

	m_rcWnd.top = 0;
	m_rcWnd.left = 0;
	m_rcSeparator.left = 0;
	m_rcView.left = 0;
	m_rcTab.left = 0;
	m_rcTab.top = 0;

	m_focusedTab = m_activeTab = m_tabs.end();
	
	m_tracked = false;

	m_btnCloseFocused = false;
	m_btnPinFocused = false;
	focus_ = false;
}

LRESULT CMyTabView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	ReCalcContentHeight();
	return 0;
}

LRESULT CMyTabView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 0;
}

LRESULT CMyTabView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	UINT cx = LOWORD(lParam), cy = HIWORD(lParam);
	
	m_rcWnd.right = cx;
	m_rcWnd.bottom = cy;
	
	m_rcSeparator.right = cx;
	m_rcView.right = cx;
	m_rcTab.right = cx;

	ReCalcLayout();
	
	if (m_activeTab != m_tabs.end())
		m_activeTab->view->SetWindowPos(NULL, m_rcView, SWP_SHOWWINDOW);

	return 0;
}

void CMyTabView::ReCalcContentHeight() {
	CClientDC dc(m_hWnd);
	HFONT oldFont = dc.SelectFont(gdiGlobal.fntTreeView);
	CRect rc;
	dc.DrawText(L"NU", 2, &rc, DT_CALCRECT);
	m_text_height = rc.bottom - rc.top;
	m_text_v_centered_offset = (c_tab_height - m_text_height) / 2 + c_dist_v;
	dc.SelectFont(oldFont);
}

void CMyTabView::ReCalcLayout() {
	LONG x = 0, y = 0;

	int pos_in_line = 0;
	int ix = 0;
	
	m_pinnedBegin = m_tabs.end();
	bool last_pinned = false;

	for (iterator it = m_tabs.begin(); it != m_tabs.end(); ++it) {
		TBDATA& tab = *it;
		tab.state.size_changed = true;
		LONG tab_width = tab_static_width() + tab.text_width;
		LONG tab_right = x + tab_width;
		
		if (tab.state.pinned) m_pinnedBegin = it;
		
		bool over_the_boundary = (pos_in_line != 0 && m_rcWnd.right < tab_right);
		
		if (over_the_boundary || (!tab.state.pinned && last_pinned)) {
			y += c_tab_height;
			pos_in_line = 0; 
			x = 0; 
			tab_right = tab_width;
		}

		tab.rcTab.left = x;
		tab.rcTab.top = y;
		tab.rcTab.right = tab_right;
		tab.rcTab.bottom = y + c_tab_height;
			
		auto const content_top = y + c_dist_v;
		auto const content_bottom = content_top + c_tab_icon_size;

		tab.rcFreeSpace.left = tab.rcTab.left + tab_text_offset() + tab.text_width + c_dist_text_btn;
		tab.rcFreeSpace.top = content_top;
		tab.rcFreeSpace.right = tab.rcFreeSpace.left + c_dist_reserved;
		tab.rcFreeSpace.bottom = content_bottom;

		tab.rcBtnPin.left = tab.rcFreeSpace.right + c_dist_text_btn;
		tab.rcBtnPin.top = content_top;
		tab.rcBtnPin.right = tab.rcBtnPin.left + c_tab_height;
		tab.rcBtnPin.bottom = content_bottom;

		tab.rcBtnClose.left = tab.rcBtnPin.right + c_dist_btns;
		tab.rcBtnClose.top = content_top;
		tab.rcBtnClose.right = tab.rcBtnClose.left + c_tab_height;
		tab.rcBtnClose.bottom = content_bottom;

		x += tab_width;
		pos_in_line++;
		last_pinned = tab.state.pinned;
	}

	m_rcSeparator.top = y + c_tab_height;
	m_rcSeparator.bottom = m_rcSeparator.top + c_separator_height;
	
	m_rcView.top = m_rcSeparator.bottom;
	m_rcView.bottom = m_rcWnd.bottom;

	m_rcTab.bottom = m_rcSeparator.bottom;
}

LRESULT CMyTabView::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return TRUE;
}

void CMyTabView::DrawCloseButton(CDCHandle dc, const CRect& rc) {
	int x = rc.left + (rc.right - rc.left) / 2;
	int y = rc.top + (rc.bottom - rc.top) / 2;

	dc.SetPixel(x, y, gdiGlobal.clrButton);
	dc.SetPixel(x + 1, y + 1, gdiGlobal.clrButton);
	dc.SetPixel(x + 2, y + 2, gdiGlobal.clrButton);
	dc.SetPixel(x + 2, y + 2, gdiGlobal.clrButton);

	dc.SetPixel(x - 1, y + 1, gdiGlobal.clrButton);
	dc.SetPixel(x - 2, y + 2, gdiGlobal.clrButton);
	dc.SetPixel(x - 2, y + 2, gdiGlobal.clrButton);

	dc.SetPixel(x - 1, y - 1, gdiGlobal.clrButton);
	dc.SetPixel(x - 2, y - 2, gdiGlobal.clrButton);
	dc.SetPixel(x - 2, y - 2, gdiGlobal.clrButton);

	dc.SetPixel(x + 1, y - 1, gdiGlobal.clrButton);
	dc.SetPixel(x + 2, y - 2, gdiGlobal.clrButton);
	dc.SetPixel(x + 2, y - 2, gdiGlobal.clrButton);
}

void CMyTabView::DrawPinButton(CDCHandle dc, const CRect& rc, bool pinned) {
	int x = rc.left + (rc.right - rc.left - 9) / 2;
	int y = rc.top + (rc.bottom - rc.top - 9) / 2;
	if (pinned)
		DrawIconEx(dc, x, y , global.GetCustomSizeIcon(NPSystemIcon::Pinned), 9, 9, 0, NULL, DI_IMAGE | DI_MASK);
	else 
		DrawIconEx(dc, x, y, global.GetCustomSizeIcon(NPSystemIcon::UnPinned), 9, 9, 0, NULL, DI_IMAGE | DI_MASK);
}

LRESULT CMyTabView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPaintDC _dc(m_hWnd);

	if (m_tabs.empty()) {
		_dc.FillRect(m_rcWnd, gdiGlobal.brBackground);
		return 0;
	}

	CMemoryDC dc(_dc, m_rcTab);

	dc.FillRect(m_rcTab, gdiGlobal.brBackground);

	HFONT oldFont = dc.SelectFont(gdiGlobal.fntTreeView);
	dc.SetBkMode(TRANSPARENT);
	
	dc.FillRect(m_activeTab->rcTab, focus_? gdiGlobal.brActiveTab : gdiGlobal.brActiveTabNF);

	if (m_focusedTab != m_tabs.end()) {
		const auto& tab = *m_focusedTab;

		if (m_focusedTab != m_activeTab)
			dc.FillRect(tab.rcTab, gdiGlobal.brFocusedTab);

		if (m_btnCloseFocused)
			dc.FillRect(tab.rcBtnClose, gdiGlobal.brFocusedTabButton);
		if (m_btnPinFocused)
			dc.FillRect(tab.rcBtnPin, gdiGlobal.brFocusedTabButton);

		if (!tab.state.pinned) DrawPinButton(dc.m_hDC, tab.rcBtnPin, false);
		DrawCloseButton(dc.m_hDC, tab.rcBtnClose);
	}

	if (m_activeTab != m_tabs.end() && m_focusedTab != m_activeTab) {
		const auto& tab = *m_activeTab;
		if (!tab.state.pinned) DrawPinButton(dc.m_hDC, tab.rcBtnPin, false);
		DrawCloseButton(dc.m_hDC, tab.rcBtnClose);
	}

	for (auto& tab : m_tabs) {
		auto icon = tab.view->GetIcon();
		DrawIconEx(dc, tab.rcTab.left + c_dist_v, tab.rcTab.top + c_dist_h * 2, icon, c_tab_icon_size, c_tab_icon_size, 0, 
			NULL, DI_IMAGE | DI_MASK);

		auto custom_color = tab.view->CustomActiveColor();
		if (custom_color) dc.FillRect(tab.rcFreeSpace, *custom_color);

		dc.TextOut(tab.rcTab.left + tab_text_offset(), tab.rcTab.top + m_text_v_centered_offset,
			tab.text.c_str(), static_cast<int>(tab.text.length()));
		
		if (tab.state.pinned) DrawPinButton(dc.m_hDC, tab.rcBtnPin, true);
	}
	
	dc.FillRect(m_rcSeparator, focus_ ? gdiGlobal.brActiveTab : gdiGlobal.brActiveTabNF);

	dc.SelectFont(oldFont);

	return 0;
}

CMyTabView::iterator CMyTabView::GetTab(CPoint pt) {
	for (iterator it = m_tabs.begin(); it != m_tabs.end(); ++it) {
		if (PtInRect(it->rcTab, pt) == TRUE)
			return it;
	}
	return m_tabs.end();
}

CMyTabView::iterator CMyTabView::GetTab(iterator begin, iterator end, CPoint pt) {
	for (iterator it = begin; it != end; ++it) {
		if (PtInRect(it->rcTab, pt) == TRUE)
			return it;
	}
	return m_tabs.end();
}

void CMyTabView::TrackMouse() {
	if (m_tracked) return;
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE;
	tme.dwHoverTime = 1;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
	m_tracked = true;
}

void CMyTabView::Drag(const CPoint& point) {
	if (!PtInRect(m_rcTab, point)) return;

	const TBDATA& tab = *m_activeTab;
	
	switch (m_dragged) {
	case Dragged::None:
		if (point.x < tab.rcTab.left) {
			m_dragged = Dragged::Left;
			m_pivot = point;
		} else if (point.x > tab.rcTab.right) {
			m_dragged = Dragged::Right;
			m_pivot = point;
		}
		break;
	case Dragged::Left:
		if (point.x >= tab.rcTab.left) m_dragged = Dragged::None;
		break;
	case Dragged::Right:
		if (point.x <= tab.rcTab.right) m_dragged = Dragged::None;
		break;
	}

	if (point.x >= tab.rcTab.left && point.x <= tab.rcTab.right) m_dragged = Dragged::None;

	if (tab.rcTab.top < point.y && tab.rcTab.bottom > point.y) {
		if (m_dragged == Dragged::Left && point.x < m_pivot.x - 5 && m_activeTab != m_tabs.begin()) {
			iterator prev = std::prev(m_activeTab);
			std::swap(*m_activeTab, *prev);
			m_focusedTab = m_activeTab = prev;
			m_pivot = point;
			goto drag_happend;
		} else if (m_dragged == Dragged::Right && point.x > m_pivot.x + 5 && 
			std::next(m_activeTab) != m_tabs.end() && tab.state.pinned == std::next(m_activeTab)->state.pinned) {
			iterator next = std::next(m_activeTab);
			std::swap(*m_activeTab, *next);
			m_focusedTab = m_activeTab = next;
			m_pivot = point;
			goto drag_happend;
		}
	} else {
		if (tab.rcTab.top > point.y && m_activeTab != m_tabs.begin()) {
			iterator it = this->GetTab(m_tabs.begin(), m_activeTab, point);
			
			if (it == m_tabs.end()) return;
			
			if (it->state.pinned == tab.state.pinned) {
				it = m_tabs.insert(it, std::move(*m_activeTab));
				m_tabs.erase(m_activeTab);
				m_focusedTab = m_activeTab = it;
				goto drag_happend;
				
			}
		} else if (std::next(m_activeTab) != m_tabs.end()) {
			iterator it = this->GetTab(m_activeTab, m_tabs.end(), point);

			if (it == m_tabs.end()) return;

			if (it->state.pinned == tab.state.pinned) {
				it = m_tabs.insert(std::next(it), std::move(*m_activeTab));
				m_tabs.erase(m_activeTab);
				m_focusedTab = m_activeTab = it;
				goto drag_happend;
			}
		}

	}

	return;

drag_happend:
	ReCalcLayout();
	InvalidateTab();
}

LRESULT CMyTabView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPoint point(lParam);
	UINT flags = static_cast<UINT>(wParam);
	
	if (m_drag == true) {
		Drag(point);
		return 0;
	}

	iterator it = GetTab(point);
	
	if (it == m_tabs.end()) return 0;
	
	TBDATA& tab = *it;

	TrackMouse();

	if (m_focusedTab != it) {
		m_focusedTab = it;
		InvalidateRect(m_rcTab, FALSE);
		return 0;
	}
	if (PtInRect(tab.rcBtnPin, point) == TRUE) {
		if (!m_btnPinFocused) {
			m_btnPinFocused = true;
			m_btnCloseFocused = false;
			InvalidateRect(m_rcTab, FALSE);
		}
		return 0;
	}
	if (PtInRect(tab.rcBtnClose, point) == TRUE) {
		if (!m_btnCloseFocused) {
			m_btnCloseFocused = true;
			m_btnPinFocused = false;
			InvalidateRect(m_rcTab, FALSE);
		}
		return 0;
	}
	if (m_btnCloseFocused || m_btnPinFocused) {
		m_btnCloseFocused = false;
		m_btnPinFocused = false;
		InvalidateRect(m_rcTab, FALSE);
		return 0;
	}
	
	return 0;
}

LRESULT CMyTabView::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	m_focusedTab = m_tabs.end();
	m_tracked = false;
	m_btnPinFocused = false;
	m_btnCloseFocused = false;
	InvalidateRect(m_rcTab, FALSE);
	return NULL;
}
void CMyTabView::PinTab(iterator it, bool pin) {
	if (m_tabs.size() == 1) {
		it->state.pinned ^= true;
		if (it->state.pinned)
			m_pinnedBegin = it;
		InvalidateTab();
		return;
	}

	if (pin == true) {
		if (it->state.pinned == true) return;

		it->state.pinned = true;

		m_pinnedBegin = m_tabs.insert(
			m_pinnedBegin == m_tabs.end() ? m_tabs.begin() : std::next(m_pinnedBegin), std::move(*it));
		
		if (m_activeTab == it) m_activeTab = m_pinnedBegin;
		
		m_tabs.erase(it);
	} else {
		if (it->state.pinned == false) return;

		it->state.pinned = false;

		iterator prev_pinned = (it == m_tabs.begin() ? m_tabs.end() : std::prev(it));
	
		m_tabs.push_back(std::move(*it));

		if (m_activeTab == it) {
			m_tabs.erase(it);
			m_activeTab = std::prev(m_tabs.end());
		} else {
			m_tabs.erase(it);
		}

		m_pinnedBegin = prev_pinned;
	}
	ReCalcLayout();
	m_activeTab->state.size_changed = false;
	m_activeTab->view->SetWindowPos(NULL, m_rcView, SWP_SHOWWINDOW);
}

LRESULT CMyTabView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	SetCapture();
	CPoint point(lParam);
	UINT flags = static_cast<UINT>(wParam);

	iterator it = GetTab(point);

	if (it == m_tabs.end())
		return 0;

	ATLASSERT(m_activeTab != m_tabs.end());

	TBDATA& tab = *it;

	if (PtInRect(tab.rcBtnPin, point) == TRUE) {
		m_focusedTab = m_tabs.end();
		PinTab(it, tab.state.pinned ^ 1);
		InvalidateRect(m_rcTab, FALSE);
		return 0;
	}
	if (PtInRect(tab.rcBtnClose, point) == TRUE) {
		RemoveTab(it);
		OnMouseMove(uMsg, wParam, lParam, bHandled);
		return 0;
	}

	ActivateTab(it);
	m_drag = true;
	
	return 0;
}
LRESULT CMyTabView::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	ReleaseCapture();
	m_drag = false;
	m_dragged = Dragged::None;
	return 0;
}
CMyTabView::iterator CMyTabView::GetIteratorFromPtr(const CView* view) noexcept {
	iterator it = m_tabs.begin();

	for (; it != m_tabs.end(); ++it) {
		if (it->view == view)
			break;
	}
	return it;
}
size_t CMyTabView::GetTabIndex(CView* view) noexcept {
	size_t ix = 0;
	for (const auto& tab : m_tabs) {
		if (tab.view == view)
			break;
		ix++;
	}

	ASSERT(ix != m_tabs.size());
	
	return ix;
}
void CMyTabView::ActivateTab(iterator it) {
	ATLASSERT(it != m_tabs.end());
	
	if (m_activeTab == it) {
		if (focus_ == false) {
			focus_ = true;
			m_activeTab->view->SetFocus();
			InvalidateRect(m_rcTab, FALSE);
		}
		return;
	}
	
	TBDATA& tab = *it;
	
	if (m_activeTab != m_tabs.end()) {
		m_activeTab->view->ShowWindow(SW_HIDE);
		m_activeTab->state.active = false;
	}

	if (tab.state.size_changed) {
		tab.view->SetWindowPos(NULL, m_rcView, SWP_SHOWWINDOW);
		tab.state.size_changed = false;
	} else {
		tab.view->ShowWindow(SW_SHOW);
	}
	
	tab.view->SetFocus();
	tab.state.active = true;

	focus_ = true;
	m_activeTab = it;
	
	InvalidateRect(m_rcTab, FALSE);
}

void CMyTabView::ActivateTab(CView* view) {
	ATLASSERT(view);

	iterator it = GetIteratorFromPtr(view);

	ATLASSERT(it != m_tabs.end());

	ActivateTab(it);
}

LONG CMyTabView::GetTextWidth(const std::wstring& text) {
	CRect rc;
	CClientDC dc(m_hWnd);
	HFONT oldFont = dc.SelectFont(gdiGlobal.fntTreeView);
	dc.DrawText(text.c_str(), static_cast<int>(text.length()), &rc, DT_CALCRECT);
	LONG width = rc.right - rc.left;
	dc.SelectFont(oldFont);
	return width;
}

void CMyTabView::AddView(CView* view, const std::string& title) {
	ATLASSERT(view);
	auto title_w = wide(title);
	m_tabs.emplace_back(view, title_w, GetTextWidth(title_w));
	ReCalcLayout();
	ActivateTab(std::prev(m_tabs.end()));
}

void CMyTabView::AddView(CItemView* view) {
	ATLASSERT(view);
	
	std::string title;
	view->GetItem()->GetViewTitle(title);
	AddView(view, title);
}

void CMyTabView::UpdateTitle(CView* view, const std::wstring& text) {
	ATLASSERT(view);

	iterator it = GetIteratorFromPtr(view);
	ATLASSUME(it != m_tabs.end());
	
	it->text = text;
	it->text_width = GetTextWidth(text);

	ReCalcLayout();
}

void CMyTabView::UpdateTitle(CView* view, const std::string& title) {
	UpdateTitle(view, wide(title));
}

void CMyTabView::DeleteView(CView* view) {
	ATLASSERT(view);

	view->DestroyWindow();
	view->PostDestroy();
	delete view;
}

void CMyTabView::RemoveTab(CView* view) {
	ATLASSERT(view);

	iterator it = GetIteratorFromPtr(view);

	ATLASSERT(it != m_tabs.end());
	
	RemoveTab(it);
}

void CMyTabView::RemoveTab(iterator it) {
	ATLASSERT(it != m_tabs.end());
	TBDATA& tab = *it;

	if (tab.view->IsModified()) {
		CDlg_DocList1 dlg(tab);
		if (dlg.DoModal() == IDCANCEL) return;
	}
	
	DeleteView(tab.view);
	
	m_focusedTab = m_tabs.end();
	
	if (it == m_activeTab) {
		if (std::next(it) != m_tabs.end())
			m_activeTab = std::next(it);
		else if (it != m_tabs.begin())
			m_activeTab = std::prev(it);
		else
			m_activeTab = m_tabs.end();
	} 
	
	it = m_tabs.erase(it);

	ReCalcLayout();

	if (m_activeTab != m_tabs.end()) {
		TBDATA& tab = *m_activeTab;
		if (tab.state.size_changed) {
			tab.view->SetWindowPos(NULL, m_rcView, SWP_SHOWWINDOW);
			tab.state.size_changed = false;
		} else {
			tab.view->ShowWindow(SW_SHOW);
		}
		tab.view->SetFocus();
		focus_ = true;
		InvalidateRect(m_rcTab, FALSE);
	} else {
		Invalidate();
	}
}

void CMyTabView::RemoveAllTabs() {
	m_focusedTab = m_activeTab = m_tabs.end();

	for (auto& tab : m_tabs)
		DeleteView(tab.view);
	
	m_tabs.clear();
	ReCalcLayout();
	Invalidate();
}

void CMyTabView::InsertSorted(SortedTabContaner& items) noexcept {
	ATLASSERT(m_tabs.size() == 0);
	
	size_t size = items.size();
	m_tabs.resize(size);

	auto activeTab = m_tabs.begin();
	auto item = items.begin();
	for (auto it = m_tabs.begin(); it != m_tabs.end(); ++it, ++item) {
		it->view = item->view;
		it->text = wide(*item->name);
		it->text_width = GetTextWidth(it->text);
		it->state.pinned = item->state.pinned;
		if (item->state.active) {
			activeTab = it;
		}
	}

	ReCalcLayout();
	ActivateTab(activeTab);
}