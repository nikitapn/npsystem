// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "listcontrolview.h"
#include "tr_item.h"

LRESULT CListControlView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CRect clRect;
	GetClientRect(&clRect);

	list_.Create(m_hWnd, clRect, NULL, WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | LVS_ICON | LVS_REPORT, 0, 3000);
	
	ListView_SetExtendedListViewStyleEx(list_.m_hWnd, 0, LVS_EX_GRIDLINES | LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);

	view_adapter_.reset(GetItem()->GetViewAdapter());

	for (auto& col : view_adapter_->m_columns) {
		list_.AddColumn(col.colName, col.len);
	}
	
	UpdateList(true);

	list_.ShowWindow(SW_SHOW);

	return 0;
}

LRESULT CListControlView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	UINT cx = LOWORD(lParam), cy = HIWORD(lParam);
	list_.MoveWindow(0, 0, cx, cy);
	return 0;
}

LRESULT CListControlView::OnDataChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	UpdateList(false);
	return 0;
}

void CListControlView::UpdateList(bool use_old) {
	if (!use_old) {
		view_adapter_.reset(GetItem()->GetViewAdapter());
	}
	
	list_.DeleteAllItems();
	
	int ix = 0;
	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(lvItem));

	auto& columns = view_adapter_->m_columns;
	auto& data = view_adapter_->m_data;

	int n = static_cast<int>(columns.size());
	for (auto& item : data) {
		lvItem.cchTextMax = 25;
		lvItem.mask = LVIF_TEXT;
		lvItem.iItem = ix;
		auto text = wide((const char*)item.values[0]);
		lvItem.pszText = text.data();

		list_.InsertItem(&lvItem);

		for (int i = 1; i < n; ++i ) {
			list_.SetItem(ix, i, LVIF_TEXT, wide(
				string_format(columns[i].format, item.values[i])
			).c_str(), 0, 0, 0, 0);
		}

		list_.SetItemData(ix, (LPARAM)item.lParam);

		ix++;
	}
}
LRESULT CListControlView::OnCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (wNotifyCode == 0 || wNotifyCode == 1) {
		ASSERT(selected_);
		REQUEST req((UINT)wID, selected_);
		selected_->HandleRequest(&req);
		selected_ = nullptr;
	}
	return 0;
}

LRESULT CListControlView::OnListNotify(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	if (pnmh->code == NM_DBLCLK) {
		LPNMITEMACTIVATE lpItem = (LPNMITEMACTIVATE)pnmh;
		if (lpItem->iItem == -1) {
			return 0;
		} else {
			auto item = (CTreeItemAbstract*)list_.GetItemData(lpItem->iItem);
			REQUEST req(ID_ITEM_PROPERTIES, item);
			item->HandleRequest(&req);
		}
	} else if (pnmh->code == NM_RCLICK) {
		LPNMITEMACTIVATE lpItem = (LPNMITEMACTIVATE)pnmh;
		CPoint ptCursor;
		GetCursorPos(&ptCursor);
		if (lpItem->iItem != -1) {
			selected_ = (InteractiveItem*)list_.GetItemData(lpItem->iItem);
		} else {
			selected_ = view_adapter_->GetItem(); // clicked on empty space
		}
		selected_->ShowMenu(ptCursor, this);
	}
	
	return 0;
}