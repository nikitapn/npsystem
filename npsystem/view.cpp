// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "tr_item.h"
#include "View.h"
#include "global.h"

void CItemView::PostDestroy() {
	item_->view_ = nullptr;
}


HICON CView::GetIcon() const noexcept { 
	return global.GetIcon32x32(NPSystemIcon::FBD); 
}
