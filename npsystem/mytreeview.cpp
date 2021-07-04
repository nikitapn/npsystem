// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "Stuff.h"
#include "GDIGlobal.h"
#include "Global.h"
#include "MyTreeView.h"
#include "tr_system.h"

#define COLOR_TREE_BACKGROUND RGB(248,248,248)
#define COLOR_SELECTED RGB(230, 255, 230)
#define COLOR_LAST_SELECTED RGB(200, 255, 230)

#define COLOR_DROP_OK RGB(30, 200, 30)
#define COLOR_DROP_BAD RGB(200, 30, 50)

#define MY_TVIS_SELECTED 0x1000
#define MY_TVIS_LAST_SELECTED 0x2000
#define MY_TVIS_DROP_OK 0x4000
#define MY_TVIS_DROP_BAD 0x8000


HWND CMyTreeView::Create(_In_opt_ HWND hWndParent,
		_In_ _U_RECT rect,
		_In_opt_z_ LPCTSTR szWindowName,
		_In_ DWORD dwStyle,
		_In_ DWORD dwExStyle,
		_In_ _U_MENUorID MenuOrID,
		_In_opt_ LPVOID lpCreateParam) 
{
	ATLASSERT(hWndParent);

	m_brButton.CreateSolidBrush(RGB(0, 0, 0));
	m_brButtonFocused.CreateSolidBrush(RGB(0, 0, 255));

	base::Create(hWndParent, rcDefault, szWindowName, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_HSCROLL | WS_TABSTOP |
		TVS_LINESATROOT | TVS_EDITLABELS | TVS_INFOTIP | TVS_FULLROWSELECT
		/*,WS_EX_CLIENTEDGE*/);

	ATLASSERT(m_hWnd);

	//  TreeView_SetExtendedStyle(m_hWnd, TVS_EX_DOUBLEBUFFER, 0);

	constexpr POINT pt_c[] = { { 2, 0 }, { 8, 5 }, { 2, 10 } };
	constexpr POINT pt_e[] = { { 2, 10 }, { 10, 2 }, { 10, 10 } };

	m_rgnButtonCol.CreatePolygonRgn(pt_c, 3, ALTERNATE);
	m_rgnButtonExp.CreatePolygonRgn(pt_e, 3, ALTERNATE);

	CTreeItemAbstract::m_treeview.Attach(m_hWnd);
	CTreeItemAbstract::m_tabview = &m_tabview;

	UpdateMetrics();

	m_imageList.Create(constants::treeview::icon_cx, constants::treeview::icon_cy, ILC_COLOR32, 1, 1);
	SetImageList(m_imageList, TVSIL_NORMAL);

	LoadTree();

	m_loaded = true;

	SetBkColor(COLOR_TREE_BACKGROUND);

	m_bDrag = 0;
	m_hLast = NULL;
	m_labelEdit = FALSE;

	m_MouseCursor = AlphaCursor::Arrow;

	m_hBackBrush = ::CreateSolidBrush(RGB(100, 100, 100));

	HTREEITEM hRoot = GetRootItem();
	m_selectedItems.insert(hRoot);
	SetItemState(hRoot, MY_TVIS_LAST_SELECTED, TVIS_USERMASK);

	return m_hWnd;
}

void CMyTreeView::UpdateMetrics() {
	SetFont(gdiGlobal.fntTreeView);
//	LOGFONT lf;
//	gdiGlobal.font.GetLogFont(lf);
	SetItemHeight(constants::treeview::label_cy);
}

inline void CMyTreeView::CreateLogicButtonRectFromTextRect(CRect& rc) {
	rc.left = rc.left - constants::treeview::icon_cx - 20 - button_cx;
	rc.right = rc.left + button_cx + 10;
	rc.top += 2; // for correct mouse move handling
	rc.bottom -= 2;
}

LRESULT CMyTreeView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	bHandled = FALSE;
	delete m_root;
	m_root = nullptr;
	return 0;
}

// Notifications

LRESULT CMyTreeView::OnTvnItemExpanding(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	CWaitCursor wc;
	NMTREEVIEW* pnmtv = (NMTREEVIEW*)pnmh;
	HTREEITEM htrItem = pnmtv->itemNew.hItem;
	LPARAM lparam = GetItemData(htrItem);
	auto item = reinterpret_cast<CContainer*>(lparam);
	return item->LoadChilds(true) ? FALSE : TRUE;
}

LRESULT CMyTreeView::OnTvnBeginLabelEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pnmh);

	if (m_fMultiSelect/* || m_lastSelected != pTVDispInfo->item.hItem */) {
		return TRUE;
	}

	LPARAM lparam = GetItemData(pTVDispInfo->item.hItem);
	auto item = (CTreeItemAbstract*)lparam;
	if (item->IsUpperCase()) {
		auto edit = GetEditControl();
		edit.ModifyStyle(0, ES_UPPERCASE);
	}

	m_labelEdit = TRUE;

	return FALSE;
}
LRESULT CMyTreeView::OnTvnEndLabelEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pnmh);
	if (pTVDispInfo->item.mask &= TVIF_TEXT) {
		LPARAM lparam = GetItemData(pTVDispInfo->item.hItem);
		auto item = (CTreeItemAbstract*)lparam;
		
		//auto locale = _get_current_locale();
		//for (auto ptr = pTVDispInfo->item.pszText; *ptr; ptr++) {
		//	*ptr = _towupper_l(*ptr, locale);
		//}
		auto text = narrow(pTVDispInfo->item.pszText);
		if (item->ChangeName(text)) {
			item->UpdateTreeLabel();
		}
	}
	m_labelEdit = FALSE;
	return FALSE;
}
LRESULT CMyTreeView::OnTvnBeginDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pnmh);
	m_hDragItemOld = pNMTreeView->itemOld.hItem;
	m_selectedItems.insert(pNMTreeView->itemNew.hItem);
	m_bDrag = TRUE;
	return FALSE;
}
LRESULT CMyTreeView::OnNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	LPNMTVCUSTOMDRAW tv = (LPNMTVCUSTOMDRAW)pnmh;
	const HTREEITEM& htr = (HTREEITEM)tv->nmcd.dwItemSpec;

	LRESULT lResult;
	CRect rc;
	UINT mystate;

	switch (tv->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		lResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		tv->clrText = RGB(0, 0, 0);
		tv->clrTextBk = COLOR_TREE_BACKGROUND;

		mystate = GetItemState(htr, TVIS_USERMASK);

		switch (mystate & TVIS_USERMASK) {
		case MY_TVIS_SELECTED:
			tv->clrTextBk = COLOR_SELECTED;
			break;
		case MY_TVIS_LAST_SELECTED:
			tv->clrTextBk = COLOR_LAST_SELECTED;
			break;
		case MY_TVIS_DROP_OK:
			tv->clrTextBk = COLOR_DROP_OK;
			break;
		case MY_TVIS_DROP_BAD:
			tv->clrTextBk = COLOR_DROP_BAD;
			break;
		default:
			if (tv->nmcd.uItemState & CDIS_FOCUS && !m_fMultiSelect) {
				tv->clrTextBk = COLOR_SELECTED;
			}
			break;
		}

		tv->nmcd.uItemState &= ~CDIS_FOCUS;

		lResult = CDRF_NOTIFYPOSTPAINT;
		break;
	case CDDS_ITEMPOSTPAINT:
		auto item = reinterpret_cast<CTreeItemAbstract*>(tv->nmcd.lItemlParam);
		GetItemRect(htr, &rc, TRUE);

		if (tv->nmcd.lItemlParam) {
			item->DrawIcon(tv->nmcd.hdc, rc.left - constants::treeview::icon_cx, rc.top + constants::treeview::icon_y_offset);
		}
		if (htr != TVI_ROOT && GetChildItem(htr) != NULL) {
			int x = rc.left - constants::treeview::icon_cx - 14 - button_cx;
			int y = rc.top + button_y_offset;

			HBRUSH brush = (m_buttonHovered == htr ? m_brButtonFocused : m_brButton);

			auto expanded = GetItemState(htr, TVIS_EXPANDED) & TVIS_EXPANDED;
			if (expanded) {
				m_rgnButtonExp.OffsetRgn(x, y);
				::FillRgn(tv->nmcd.hdc, m_rgnButtonExp, brush);
				m_rgnButtonExp.OffsetRgn(-x, -y);
			} else {
				m_rgnButtonCol.OffsetRgn(x, y);
				::FrameRgn(tv->nmcd.hdc, m_rgnButtonCol, brush, 1, 1);
				m_rgnButtonCol.OffsetRgn(-x, -y);
			}
		}

		lResult = CDRF_SKIPDEFAULT;
		break;
	};
	return lResult;
}

LRESULT CMyTreeView::OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
//	m_lastSelected = NULL;

	CPoint point(lParam);
	HTREEITEM htr = HitTest(point, NULL);

	if (htr == NULL)
		return 0;

	CRect rc;
	GetItemRect(htr, &rc, TRUE);

	CreateLogicButtonRectFromTextRect(rc);

	if (PtInRect(rc, point)) {
		Expand(htr, TVE_TOGGLE);
		return 0;
	}

	auto item = (CTreeItemAbstract*)GetItemData(htr);
	item->ShowView();

	return 0;
}

LRESULT CMyTreeView::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	HTREEITEM htr = GetSelectedItem();

	if (htr == NULL)
		return 0;

	// EditLabel(htr);
	//	KillTimer(1);
	return 0;
}

void CMyTreeView::LoadTree() {
	auto& state = global.controls.tree_state;
	const auto& opened_windows = global.controls.opened_windows;

	CMyTabView::SortedTabContaner windows;
	windows.reserve(opened_windows);

	std::vector<CTreeItemAbstract*> expanded;
	expanded.reserve(1000);

	npsys::system_n system;
	if (!system.fetch() && !system.is_connection_loss()) {
		system.create(new npsys::CSystem("SYSTEM"));
	} 
	if (system.is_invalid_node()) {
		ASSERT(FALSE);
		std::abort();
	}
	m_root = new CTreeSystem(system);
	Iter::PreorderIterator<CTreeItemAbstract*> it(m_root);

	for (; !it.IsDone(); ) {
		auto item = (*it);
		oid_t id = item->GetId();
		if (state[id].expanded) {
			static_cast<CContainer*>(item)->LoadChilds();
			expanded.push_back(item);
		}
		if (state[id].window) {
			try {
				auto view = item->CreateView(m_tabview);
				windows.emplace_back(view, &item->GetName(), state[id]);
				item->SetWindow(view);
			} catch (...) {}
		}
		if (state[id].expanded) it.Next();
		else it.NextSkipBranch();
	}

	m_root->Insert(TVI_ROOT);
	Expand(m_root->GetHTR(), TVE_EXPAND);

	for (auto item : expanded) {
		Expand(item->GetHTR(), TVE_EXPAND);
	}

	if (opened_windows > 0 && windows.size() != 0) {
		using Value = CMyTabView::SortedTabContaner::value_type;
		std::sort(windows.begin(), windows.end(), [](const Value& a, const Value& b) {
			return a.state.index < b.state.index;
		});
		m_tabview.InsertSorted(windows);
	}

	m_root->CalcStatusAll();
}

void CMyTreeView::SaveTree() {
	auto& state = global.controls.tree_state;
	auto& opened_windows = global.controls.opened_windows;

	state.clear();
	opened_windows = 0;

	Iter::PreorderIterator<CTreeItemAbstract*> it(m_root);
	for (; !it.IsDone(); it.Next()) {
		auto item = *it;

		TreeItemState item_state;
		item_state.window = item->IsWindow();
		item_state.expanded = (GetItemState(item->GetHTR(), TVIS_EXPANDED) & TVIS_EXPANDED) ? true : false;
		if (item->IsWindow()) {
			item_state.window = true;
			auto tab_item_state = m_tabview.GetItemState(item->GetWindow());
			item_state.pinned = tab_item_state.pinned;
			item_state.index = m_tabview.GetTabIndex(item->GetWindow());
			item_state.active = tab_item_state.active;
			opened_windows++;
		}
		state[item->GetId()] = item_state;
	}
}

void CMyTreeView::InvalidateItem(CTreeItemAbstract* pItem) {
	CRect rect;
	GetItemRect(pItem->GetHTR(), &rect, false);
	InvalidateRect(&rect);
}

LRESULT CMyTreeView::OnCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	HTREEITEM htr = GetSelectedItem();
	CTreeItemAbstract* pItem = (CTreeItemAbstract*)GetItemData(htr);

	REQUEST req(wID, pItem);
	pItem->HandleRequest(&req);

	return 0;
}

LRESULT CMyTreeView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	SetFocus();
	CPoint pt(lParam);
	UINT flags = LOWORD(wParam);

	if (flags & MK_CONTROL) {
		//	DBG_PRINT("WM_LBUTTONDOWN + MK_CONTROL\n\tm_fMultiSelect:%d\n", m_fMultiSelect);

		if (!m_fMultiSelect) {
			HTREEITEM hItemPrev = GetSelectedItem();
			if (hItemPrev != NULL) {
				SetItemState(hItemPrev, MY_TVIS_SELECTED, TVIS_USERMASK);
				m_selectedItems.insert(hItemPrev);
			}
		}

		m_fMultiSelect = 1;

		HTREEITEM hItem = HitTest(pt, NULL);

		if (hItem == NULL) return 0;

		auto it = m_selectedItems.find(hItem);
		if (it == m_selectedItems.cend()) {
			if (!m_selectedItems.empty() &&
				GetParentItem(*m_selectedItems.begin()) != GetParentItem(hItem)) {
				return FALSE;
			}
			m_selectedItems.insert(hItem);
			SetItemState(hItem, MY_TVIS_SELECTED, TVIS_USERMASK);
		} else {
			m_selectedItems.erase(hItem);
			SetItemState(hItem, 0, TVIS_USERMASK);
		}
		return FALSE;
	} else if (wParam & MK_SHIFT) {

	} else {
		CPoint pt(lParam);
		HTREEITEM hItem = HitTest(pt, NULL);

		if (hItem) {
			CRect rc;
			GetItemRect(hItem, &rc, TRUE);
			CreateLogicButtonRectFromTextRect(rc);

			if (PtInRect(rc, pt)) {
				Expand(hItem, TVE_TOGGLE);
				return 0;
			}

			if (m_fMultiSelect) {
				if (m_selectedItems.find(hItem) != m_selectedItems.end()) {
				//	bHandled = FALSE;
				} else {
					UnSelect();
					m_selectedItems.insert(hItem);
					SelectItem(hItem);
				}
			} else if (m_selectedItems.size() == 1 && *m_selectedItems.begin() == hItem) {
				bHandled = FALSE; // OnTvnBeginDrag wil not work otherwise
			} else {
				UnSelect();
				SelectItem(hItem);
				m_selectedItems.insert(hItem);
				bHandled = FALSE; // OnTvnBeginDrag wil not work otherwise
			}
		} else {
			UnSelect();
			SelectItem(NULL);
		}
	}
	return 0;
}
LRESULT CMyTreeView::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (m_bDrag) {
		if (m_bDragResult) {
			for (auto& i : m_selectedItems) {
				CTreeItemAbstract* pItem = reinterpret_cast<CTreeItemAbstract*>(GetItemData(i));
				pItem->Move(m_pDestItem, 1);
			}
		}

		if (m_hDragItemOld != NULL) {
			SetItemState(m_hDragItemOld, 0, TVIS_USERMASK);
		}

		m_hDragItemOld = NULL;
		m_bDrag = 0;

		m_MouseCursor = AlphaCursor::Arrow;
		SetCursor(global.GetCursor(m_MouseCursor));

		return 0;
	} else {
		bHandled = FALSE;
	}
	return 0;
}
LRESULT CMyTreeView::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	SetFocus();
	UINT uFlags = 0;
	CPoint pt(lParam);
	HTREEITEM hItem = HitTest(pt, &uFlags);
	if (hItem != NULL) {
		int state = GetItemState(hItem, TVIS_USERMASK) & TVIS_USERMASK;
		if (state != MY_TVIS_SELECTED) {
			UnSelect();
			if (GetSelectedItem() != hItem) {
				SelectItem(hItem);
			}
			CTreeItemAbstract *pItem = reinterpret_cast<CTreeItemAbstract*>(GetItemData(hItem));
			ASSERT(pItem);
			ClientToScreen(&pt);
			pItem->ShowMenu(pt, this);
		} else {
			UnSelect();
			CTreeItemAbstract *pItem = reinterpret_cast<CTreeItemAbstract*>(GetItemData(hItem));
			ASSERT(pItem);
			ClientToScreen(&pt);
			pItem->ShowMenu(pt, this);
		}
	}
	return 0;
}
LRESULT CMyTreeView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	switch (wParam) {
	case VK_F2: 
	{
		if (m_selectedItems.size() == 1) {
			auto item = *m_selectedItems.begin();
			EditLabel(item);
		}
		break;
	}
	case VK_RETURN:
		EndEditLabelNow(FALSE);
		break;
	case VK_ESCAPE:
		EndEditLabelNow(TRUE);
		break;
	default:
		break;
	}
	return 0;
}
LRESULT CMyTreeView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPoint point(lParam);
	UINT flags = static_cast<UINT>(wParam);

	HTREEITEM hItem = HitTest(point, NULL);

	if (flags & MK_LBUTTON && m_bDrag) {
		if (hItem) {
			m_pDestItem = reinterpret_cast<CTreeItemAbstract*>(GetItemData(hItem));

			if (m_hDragItemOld == hItem) {
				bHandled = FALSE;
				return 0;
			}

			if (m_selectedItems.find(hItem) != m_selectedItems.end()) {
				bHandled = FALSE;
				m_bDragResult = false;
				return 0;
			}

			if (m_hDragItemOld) {
				SetItemState(m_hDragItemOld, 0, TVIS_USERMASK);
			}

			m_hDragItemOld = hItem;

			for (const auto htr : m_selectedItems) {
				auto item = reinterpret_cast<CTreeItemAbstract*>(GetItemData(htr));
				if ((m_bDragResult = item->Move(m_pDestItem, 0)) == 0)
					break;
			}
			if (m_bDragResult) {
				m_MouseCursor = AlphaCursor::Drop;
				SetItemState(hItem, MY_TVIS_DROP_OK, TVIS_USERMASK);
			} else {
				m_MouseCursor = AlphaCursor::Block;
				SetItemState(hItem, MY_TVIS_DROP_BAD, TVIS_USERMASK);
			}
		} else {
			m_pDestItem = NULL;
			m_bDragResult = 0;
		}
	} else {
		m_MouseCursor = AlphaCursor::Arrow;

		if (hItem == NULL) {
			if (m_buttonHovered != NULL) {
				CRect rc;
				GetItemRect(hItem, &rc, FALSE);
				m_buttonHovered = NULL;
				InvalidateRect(rc);
			}
			return 0;
		}

		CRect rc;
		GetItemRect(hItem, &rc, TRUE);
		CreateLogicButtonRectFromTextRect(rc);

		if (PtInRect(rc, point)) {
			if (m_buttonHovered != hItem) {
				m_buttonHovered = hItem;

				CRect prev;
				GetItemRect(m_buttonHovered, &prev, FALSE);
				prev.top = std::min(prev.top, rc.top);
				prev.bottom = std::max(prev.bottom, rc.bottom);
				InvalidateRect(prev);
			}
		} else {
			if (m_buttonHovered != NULL) {
				CRect prev;
				GetItemRect(m_buttonHovered, &prev, FALSE);
				m_buttonHovered = NULL;
				InvalidateRect(prev);
			}
		}
	}
	return 0;
}

void CMyTreeView::UnSelect() {
	for (auto& i : m_selectedItems) {
		SetItemState(i, 0, TVIS_USERMASK);
	}
	m_selectedItems.clear();
	m_fMultiSelect = 0;
}

LRESULT CMyTreeView::OnDeleteItem(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	auto delete_one_item = [&](HTREEITEM htr) -> bool {
		auto item = (CTreeItemAbstract*)GetItemData(htr);
		if (!item) return false;
		if (item->IsRemovable() &&
			(::MessageBoxA(m_hWnd, ("Delete \"" + item->GetName() + "\"?").c_str(), "",
				MB_OKCANCEL) == IDOK)
			) {
			item->Delete();
			return true;
		}
		return false;
	};

	if ((wID == ID_ITEM_DELETE) || (m_selectedItems.size() == 1)) {
		auto htr = ((wID == ID_ITEM_DELETE) ? GetSelectedItem() : *m_selectedItems.begin());
		auto next = GetNextItem(htr, TVGN_NEXTVISIBLE);
		if (delete_one_item(htr)) {
			m_selectedItems.clear();
			m_selectedItems.emplace(next);
			SelectItem(next);
		}
		return 0;
	}

	if (m_selectedItems.size() == 0) return 0;

	for (auto item_htr : m_selectedItems) {
		auto item = reinterpret_cast<CTreeItemAbstract*>(GetItemData(item_htr));
		if (item->IsRemovable() == false) {
			return 0;
		}
	}

	if (::MessageBoxA(m_hWnd, "Delete selected items?", "", MB_OKCANCEL) != IDOK) return 0;

	for (auto item_htr : m_selectedItems) {
		auto item = reinterpret_cast<CTreeItemAbstract*>(GetItemData(item_htr));
		item->Delete();
	}

	m_selectedItems.clear();
	m_fMultiSelect = false;
	SelectItem(NULL);

	return 0;
}

LRESULT CMyTreeView::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// SelectItem() is forbidden here
	bHandled = FALSE;
	for (auto& i : m_selectedItems) {
		SetItemState(i, MY_TVIS_LAST_SELECTED, TVIS_USERMASK);
	}

	return 0;
}

LRESULT CMyTreeView::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	bHandled = FALSE;
	for (auto& i : m_selectedItems)
		SetItemState(i, MY_TVIS_SELECTED, TVIS_USERMASK);

	return 0;
}

LRESULT CMyTreeView::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (m_bDrag) {
		SetCursor(global.GetCursor(m_MouseCursor));
		return TRUE;
	} 
		
	bHandled = FALSE;
	return 0;
}