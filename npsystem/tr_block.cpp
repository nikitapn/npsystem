// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "tr_block.h"
#include "block.h"
#include "dlgparameter.h"

void CTreeBlock::LoadChildsImpl() noexcept {
	n_->slots.fetch_all_nodes();
	for (auto& i : n_->slots) {
		insert(new CTreeBlockValue(i));
	}
}

void CTreeBlockValue::ConstructMenu(CMenu* menu) noexcept {
	menu->AppendMenu(MF_STRING, ID_ITEM_PROPERTIES, L"Propetries");
	//	menu->AppendMenu(MF_STRING, ID_CREATE_LINK_FROM_PIN, L"Copy As Link");
};

INT_PTR CTreeBlockValue::ShowProperties() {
	CDlg_Parameter dlg(n_);
	auto result = dlg.DoModal();
	return n_.is_new_node() ? IDCANCEL : result;
}

void CTreeBlockValue::HandleRequest(REQUEST* req) noexcept {
	switch (req->ID_MENU) {
	case ID_CREATE_LINK_FROM_PIN:
		break;
	default:
		__super::HandleRequest(req);
		break;
	};
}