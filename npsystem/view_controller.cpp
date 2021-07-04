// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "tr_controller.h"
#include "view_controller.h"
#include "avrassigned.h"

CControllerView::CControllerView(CTreeController* item, CMyTabView& tabview)
	: base(item, tabview) {

}

LRESULT CControllerView::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	m_cbAdvise = GetDlgItem(IDC_BUTTON1);

	m_editAddress.SubclassWindow(GetDlgItem(IDC_EDIT1));
	m_editModel.SubclassWindow(GetDlgItem(IDC_EDIT2));
	m_editVersion.SubclassWindow(GetDlgItem(IDC_EDIT3));
	m_listLibs = GetDlgItem(IDC_LIST1);

	auto item = static_cast<CTreeController*>(item_);
	
	auto avr = item->n_.cast<npsys::controller_n_avr>();

	::SetWindowTextA(m_editModel, avrinfo::controller_model_to_string(item->n_->controller_model).c_str());
	::SetWindowTextA(m_editAddress, std::to_string(item->n_->dev_addr).c_str());
	::SetWindowTextA(m_editVersion, std::to_string(avr->version).c_str());

	auto wrapper = GetDlgItem(IDC_TREE1);
	
	CRect rcWrapper;
	wrapper.GetWindowRect(rcWrapper);
	ScreenToClient(&rcWrapper);
	
	tree_.Create(m_hWnd, rcWrapper);
	tree_.CreateItems(item_);
	
	static constexpr std::tuple<const wchar_t*, int> listItems[] = { 
		{L"library", 150}, 
		{L"object", 150},
		{L"page_begin", 120},
		{L"page_end", 100},
		{L"symbols", 500} 
	};
	
	int column = 0;
	LVCOLUMN lvColumn;

	lvColumn.cchTextMax = 50;
	lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;
	for (const auto& item : listItems) {
		lvColumn.pszText = const_cast<wchar_t*>(std::get<0>(item));
		lvColumn.cx = std::get<1>(item);
		m_listLibs.InsertColumn(column++, &lvColumn);
	}

	avr->assigned_ofs.fetch_all_nodes();
	
	int ix = 0;

	LVITEMW lvItem = {};
	lvItem.mask = LVIF_TEXT;
	
	for (auto& lib : avr->assigned_ofs) {
		auto text = wide(lib->desc.lib);
		lvItem.iItem = ix;
		lvItem.iSubItem = 0;
		lvItem.pszText = text.data();
		
		::SendMessage(m_listLibs, LVM_INSERTITEM, 0, (LPARAM)&lvItem);

		lvItem.iSubItem = 1;
		text = wide(lib->desc.obj);
		lvItem.pszText = text.data();

		::SendMessage(m_listLibs, LVM_SETITEM, 0, (LPARAM)&lvItem);

		lvItem.iSubItem = 2;
		text = std::to_wstring(lib->Start());
		lvItem.pszText = text.data();

		::SendMessage(m_listLibs, LVM_SETITEM, 0, (LPARAM)&lvItem);

		lvItem.iSubItem = 3;
		text = std::to_wstring(lib->End());
		lvItem.pszText = text.data();

		::SendMessage(m_listLibs, LVM_SETITEM, 0, (LPARAM)&lvItem);

		std::string tmp;
		size_t ix = 0;
		for (const auto& symbol : lib->expf) {
			if (ix++ != 0) tmp += ", "sv;
			tmp += symbol.name + '=' + to_hex(static_cast<uint16_t>(symbol.addr));
		}

		lvItem.iSubItem = 4;
		text = wide(tmp);
		lvItem.pszText = text.data();

		::SendMessage(m_listLibs, LVM_SETITEM, 0, (LPARAM)&lvItem);
	}

	return 0;
}

LRESULT CControllerView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	tree_.UnAdvise();
	return 0;
}

LRESULT CControllerView::OnBnClickedButtonAdvise(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (tree_.IsAdvised()) {
		tree_.UnAdvise();
		m_cbAdvise.SetWindowText(L"Advise");
		tree_.tree.Invalidate();
		return 0;
	}
	if (tree_.Advise()) {
		m_cbAdvise.SetWindowText(L"Unadvise");
	}
	return 0;
}

bool CControllerView::IsModified() {
	return 0;
}

void CControllerView::Save() {

}