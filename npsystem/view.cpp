#include "stdafx.h"
#include "tr_item.h"
#include "View.h"

void CItemView::PostDestroy() {
	item_->view_ = nullptr;
}