#include "stdafx.h"
#include "myonlinetreectrl.h"
#include "config.h"
#include "tr_item.h"
#include <npsys/variable.h>
#include <npsys/corba.h>

CMyOnlineTreeView::CMyOnlineTreeView() 
	: header_(tree, items_)
	, tree(*this, items_)
{
}

HWND CMyOnlineTreeView::Create(CWindow parent, CRect& rect) {
	base::Create(parent, rect, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER);
	ASSERT(m_hWnd);

	auto width = rect.Width();
	auto height = rect.Height();
	
	CRect rc;

	rc.left = 0;
	rc.right = width;
	rc.top = 0;
	rc.bottom = height;

	header_.Create(m_hWnd, rc, NULL, WS_CHILD | WS_VISIBLE);
	ASSERT(header_);

	rc.top = header_height;
	rc.bottom = height;
	wrapper_.Create(header_, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN);
	ASSERT(wrapper_);
	
	rc.top = 0;
	rc.bottom -= header_height;
	tree.Create(wrapper_, rc);
		
	ASSERT(tree);

	return m_hWnd;
}
LRESULT CMyOnlineTreeView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	UINT cx = LOWORD(lParam), cy = HIWORD(lParam);
	return 0;
}

void CMyOnlineTreeView::CreateItems(CTreeItemAbstract* root) {
//	using Iterator = Iter::Iterator<CTreeItemAbstract*>;
	auto ptr = root->CreateOnlineFromThis(nodes_, tree, TVI_ROOT);
	/*
	Iterator* current = root->CreateIterator();
	HTREEITEM current_parent = ptr->Insert(tree, TVI_ROOT);
	nodes_.push_back(std::move(ptr));
	root->CreateOnlineCustomItems(tree, current_parent, nodes_);

	std::stack<std::pair<Iterator*, HTREEITEM>> iterators;
	
	while (true) {
		if (current->IsDone()) {
			if (iterators.empty()) break;
			current = iterators.top().first;
			current_parent = iterators.top().second;
			iterators.pop();
		} else {
			auto item = current->CurrentItem();
			auto ptr = item->CreateOnlineFromThis(std::vector<std::unique_ptr<COnlineTreeItem>>& nodes, CTreeViewCtrl tree, HTREEITEM hParent));

			auto htr = ptr->Insert(tree, current_parent);
			iterators.emplace(item->CreateIterator(), htr);
			nodes_.push_back(std::move(ptr));
			current->Next();
		}
	}*/
	
}

static npsys::variable::Type size2type(uint8_t size) {
	switch (size)
	{
	case 1:
		return npsys::variable::Type::VT_BYTE;
	case 2:
		return npsys::variable::Type::VT_WORD;
	case 4:
		return npsys::variable::Type::VT_DWORD;
	default:
		ATLASSERT(FALSE);
		return npsys::variable::Type::VT_UNDEFINE;
	}
}

size_t CMyOnlineTreeView::AdviseImpl() {
	m_ref.clear();
	online_values_.clear();
	
	online_values_.reserve(nodes_.size());
	for (auto& val : nodes_) {
		if (val->dev_addr != 0xFF) {
			online_values_.push_back(val.get());
		}
	}

	if (!online_values_.size()) return 0;

	std::vector<nps::DataDef> a(online_values_.size());

	for (size_t i = 0; i < online_values_.size(); ++i) {
		a[i].dev_addr = online_values_[i]->dev_addr;
		a[i].mem_addr = online_values_[i]->addr;
		if (online_values_[i]->vt == COnlineTreeItem::VAR) {
			a[i].type = online_values_[i]->online.get_type();
		} else {
			a[i].type = size2type(online_values_[i]->size);
		}
	}

	{
		nprpc::Object* obj;
		npsys_rpc->server->CreateItemManager(obj);
		item_manager_.reset(nprpc::narrow<nps::ItemManager>(obj));
		assert(item_manager_);
		assert(item_manager_->policy_lifespan() == nprpc::Policy_Lifespan::Transient);
		if (!item_manager_) return 0;
		item_manager_->add_ref();
	}

	auto this_oid = npsys_rpc->callback_poa->activate_object(this);
	item_manager_->Activate(this_oid);

	std::vector<nps::var_handle> handles;
	item_manager_->Advise(flat::make_read_only_span(a), handles);

	for (int i = 0; i < online_values_.size(); ++i) {
		m_ref[handles[i]] = online_values_[i];
	}

	return online_values_.size();
}

void CMyOnlineTreeView::OnDataChangedImpl(::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> values) {
	//const auto size = a.size();

	for (auto val : values) {
		auto item = m_ref[val.h()];
		
		if (!item) return;

		if (val.s() != nps::var_status::VST_DEVICE_NOT_RESPONDED) {
			switch (item->vt)
			{
			case COnlineTreeItem::Type::BOOL:
				item->online_st = std::string("0b") + to_bit(val.data()[0]);
				break;
			case COnlineTreeItem::Type::VAR:
				item->online = val.__data();
				item->online_st = item->online.to_string_show_quality(3);
				break;
			case COnlineTreeItem::Type::HEX_UI1:
				item->online_st = to_hex(*(uint8_t*)val.data().data());
				break;
			case COnlineTreeItem::Type::HEX_UI2:
				item->online_st = to_hex(*(uint16_t*)val.data().data());
				break;
			case COnlineTreeItem::Type::HEX_UI4:
				item->online_st = to_hex(*(uint32_t*)val.data().data());
				break;
			case COnlineTreeItem::Type::FUNCTION:
				item->fn(item->online_st, *(uint32_t*)val.data().data());
				break;
			default:
				ASSERT(FALSE);
				break;
			}
		} else {
			item->online_st = "x";
		}
		RECT rcItem;
		tree.GetItemRect(item->htr, &rcItem, FALSE);
		tree.InvalidateRect(&rcItem);
	}
}

void CMyOnlineTreeView::OnConnectionLost() noexcept {
	for (auto value : online_values_) {
		value->online_st = "x";
	}
	tree.Invalidate();
}

LRESULT CMyOnlineTreeView::CHeaderView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CRect rc;
	GetClientRect(rc);

	auto width = rc.Width();

	rc_header_ = CRect(0, 0, width, CMyOnlineTreeView::header_height);

	Item item;
	item.rc.top = 0;
	item.rc.bottom = CMyOnlineTreeView::header_height;

	item.name = L"Type";
	item.SetPos(50, width);
	items_[0] = item;

	item.name = L"Value";
	item.SetPos(70, width);
	items_[1] = item;

	pen_.CreatePen(0, 1, RGB(230, 230, 230));

	return 0;
}
LRESULT CMyOnlineTreeView::CHeaderView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	pen_.DeleteObject();
	return 0;
}
LRESULT CMyOnlineTreeView::CHeaderView::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return TRUE;
}
LRESULT CMyOnlineTreeView::CHeaderView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {


	return 0;
}


LRESULT CMyOnlineTreeView::CHeaderView::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	SetCursor(global.GetCursor(cursor_));
	return 0;
}


size_t CMyOnlineTreeView::CHeaderView::GetSelected(CPoint& pt) const noexcept {
	size_t founded = -1, ix = 0;
	for (auto& item : items_) {
		if (PtInRect(item.rc, pt)) {
			founded = ix;
			break;
		}
		ix++;
	}
	return founded;
}
LRESULT CMyOnlineTreeView::CHeaderView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPoint pt(lParam);
	drag_ix_ = GetSelected(pt);
	if (drag_ix_ != -1) {
		SetCapture();
		CRect rc;
		GetClientRect(rc);
		height_ = rc.Height();
		prev_pos_ = pt.x;
		CClientDC dc(m_hWnd);
		int x = items_[drag_ix_].pos;
		diff_ = x - prev_pos_;
		dc.SelectPen(pen_);
		dc.SetROP2(R2_XORPEN);
		dc.MoveTo(prev_pos_ + diff_, 0);
		dc.LineTo(prev_pos_ + diff_, height_);
	}
	return 0;
}

LRESULT CMyOnlineTreeView::CHeaderView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPoint pt(lParam);
	if (drag_ix_ != -1) {
		CClientDC dc(m_hWnd);
		items_[drag_ix_].SetPosDx(pt.x - prev_pos_);
		dc.SelectPen(pen_);
		dc.SetROP2(R2_XORPEN);
		dc.MoveTo(prev_pos_ + diff_, 0);
		dc.LineTo(prev_pos_ + diff_, height_);
		dc.MoveTo(pt.x + diff_ , 0);
		dc.LineTo(pt.x + diff_, height_);
		
		prev_pos_ = pt.x;
	} else {
		bool cursor_set = false;
		if (GetSelected(pt) != -1) {
			cursor_ = AlphaCursor::SizeWE;
			SetCursor(global.GetCursor(AlphaCursor::SizeWE));
		} else if (cursor_ != AlphaCursor::Arrow) {
			cursor_ = AlphaCursor::Arrow;
			SetCursor(global.GetCursor(AlphaCursor::Arrow));
		}
	}

	return 0;
}

LRESULT CMyOnlineTreeView::CHeaderView::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (drag_ix_ != -1) {
		ReleaseCapture();

		CClientDC dc(m_hWnd);
		dc.SelectPen(pen_);
		dc.SetROP2(R2_XORPEN);
		dc.MoveTo(prev_pos_ + diff_, 0);
		dc.LineTo(prev_pos_ + diff_, height_);

		
		InvalidateRect(rc_header_);
		tree_.Invalidate();
		
		drag_ix_ = -1;
	}
	return 0;
}

LRESULT CMyOnlineTreeView::CHeaderView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CPaintDC dc(m_hWnd);
	CRect rc;
	GetClientRect(rc);

	CRect rcItem;
	rcItem.top = 0;
	rcItem.bottom = CMyOnlineTreeView::header_height;
	rcItem.left = rc.left + 2;
	rcItem.right = rc.right;

	dc.FillSolidRect(rcItem, RGB(255, 255, 255));

	static const auto name = L"Item";
	dc.DrawText(name, sizeof(name) / sizeof(name[0]), rcItem, DT_SINGLELINE | DT_VCENTER | DT_LEFT);

	for (auto& item : items_) {
		dc.MoveTo(item.pos, 0);
		dc.LineTo(item.pos, CMyOnlineTreeView::header_height);
		rcItem.left = item.rc.right;
		rcItem.right = rcItem.left + 100;
		dc.DrawText(item.name.c_str(), static_cast<int>(item.name.length()), rcItem, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
	}

	dc.MoveTo(0, CMyOnlineTreeView::header_height-1);
	dc.LineTo(rc.right, CMyOnlineTreeView::header_height-1);

	return 0;
}

CMyOnlineTreeView::CActualTreeView::CActualTreeView(CMyOnlineTreeView& main, const item_list& items)
	: main_(main)
	, items_(items) 
{
	line_color_selected_ = RGB(240, 240, 240);

}


HWND CMyOnlineTreeView::CActualTreeView::Create(HWND hParent, CRect& rc) {
	base::Create(hParent, rc, NULL,
		WS_CHILD | WS_VISIBLE | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_FULLROWSELECT | WS_HSCROLL);
	
	ASSERT(m_hWnd);

	SetItemHeight(constants::treeview::label_cy);

	return m_hWnd;
}

LRESULT CMyOnlineTreeView::CActualTreeView::OnTvnItemExpanding(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	CWaitCursor wc;
	NMTREEVIEW* pnmtv = (NMTREEVIEW*)pnmh;
	HTREEITEM htrItem = pnmtv->itemNew.hItem;
	LPARAM lparam = GetItemData(htrItem);
	auto item = reinterpret_cast<COnlineTreeItem*>(lparam);


	if (item->container && !item->childs_loaded) {
		DeleteItem(item->htrFake);
		item->htrFake = NULL;
		item->container->LoadChilds();
		auto it = item->container->CreateIterator();
		for (; !it->IsDone(); it->Next()) {
			it->CurrentItem()->CreateOnlineFromThis(main_.nodes_, m_hWnd, item->htr);
		}
		item->childs_loaded = true;
	}
	return FALSE;
}


LRESULT CMyOnlineTreeView::CActualTreeView::OnNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	LPNMTVCUSTOMDRAW tv = (LPNMTVCUSTOMDRAW)pnmh;
	const HTREEITEM& htr = (HTREEITEM)tv->nmcd.dwItemSpec;

	LRESULT lResult = 0;
	CRect rcItem;

	switch (tv->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		lResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		tv->clrText = RGB(0, 0, 0);
		if (tv->nmcd.uItemState & CDIS_FOCUS) {
			tv->clrTextBk = line_color_selected_;
		} else {
			tv->clrTextBk = RGB(255, 255, 255);
		}

	//	tv->nmcd.uItemState &= ~CDIS_FOCUS;

		lResult = CDRF_NOTIFYPOSTPAINT;
		break;
	case CDDS_ITEMPOSTPAINT:
		auto item = reinterpret_cast<COnlineTreeItem*>(tv->nmcd.lItemlParam);
		GetItemRect(htr, &rcItem, TRUE);
	//	rcItem = tv->nmcd.rc;
	//	rcItem.left += tv->iLevel * 20 + 24;
		CDCHandle dc(tv->nmcd.hdc);
		dc.SetBkColor(tv->clrTextBk);
	
		DrawIconEx(dc.m_hDC, rcItem.left, rcItem.top + 2, item->icon,
			constants::treeview::icon_cx, constants::treeview::icon_cy, 0, NULL, DI_IMAGE | DI_MASK);

		rcItem.left += constants::treeview::icon_cx + 2;
		rcItem.right = tv->nmcd.rc.right;

		auto text = wide(item->name);
		dc.DrawText(text.c_str(), static_cast<int>(text.length()), rcItem, DT_SINGLELINE | DT_VCENTER | DT_LEFT);

		if (main_.IsAdvised() && item->dev_addr != 0xFF) {
			rcItem.left = items_[1].pos;
			rcItem.right = rcItem.left + 150;
				auto text = wide(item->online_st);
			dc.DrawText(text.c_str(), static_cast<int>(text.length()), rcItem, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
		}

		lResult = 0;
		break;
	};
	return lResult;
}

LRESULT CMyOnlineTreeView::CActualTreeView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 0;
}