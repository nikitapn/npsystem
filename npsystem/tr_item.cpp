// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "View.h"
#include "ListControlView.h"
#include "tr_item.h"
#include "onlinetreeitem.h"
//#include "FilesContainer.h"

CTreeViewCtrl CTreeItemAbstract::m_treeview;
CMyTabView* CTreeItemAbstract::m_tabview;


// class InteractiveItem
void InteractiveItem::ShowMenu(CPoint& pt, CWindow* pWnd) {
		CMenu menu;
		menu.CreatePopupMenu();
		ConstructMenu(&menu);
		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, pWnd->m_hWnd);
}

void InteractiveItem::DefaultMenu(CMenu* menu) const noexcept {
	menu->AppendMenu(MF_STRING, ID_ITEM_DELETE, L"Delete");
	menu->AppendMenu(MF_SEPARATOR);
	menu->AppendMenu(MF_STRING, ID_ITEM_PROPERTIES, L"Properties");
}

// class CTreeItemAbstract

CTreeItemAbstract::IIteratorPtr CTreeItemAbstract::CreateIterator() {
	return new Iter::NullIterator<CTreeItemAbstract*>;
}

void CTreeItemAbstract::HandleRequest(REQUEST* req) noexcept {
	switch (req->ID_MENU) {
		case ID_SERIALIZE:
		case ID_GET_CATEGORY_KEEPER:
		case ID_ADD_EXIST:
		case ID_CREATE_FILE: {
			auto parent = GetParent();
			if (parent) parent->HandleRequest(req);
			break;
		}
		case ID_ITEM_DELETE:
			if (IsRemovable() &&
				(MessageBoxA(g_hMainWnd, ("Delete \"" + GetName() + "\"?").c_str(), "", MB_OKCANCEL) == IDOK)) {
				Delete();
			}
			break;
		case ID_ITEM_PROPERTIES: {
			auto result = ShowProperties();
			req->result = result;
			if (result == IDOK) {
				StoreItem();
				UpdateListView();
			} 
			break;
		}
		default: 
			std::cout << "Unknown request: " << req->ID_MENU << std::endl;
			break;
	};
}

void CTreeItemAbstract::Delete() noexcept {
	auto parent = GetParent();
	ATLASSERT(parent);
	
	CloseView();
	
	parent->EraseItem(this);
	
	PostDelete(parent);
	UpdateListView();
	Detach();

	delete this;
}

void CTreeItemAbstract::ShowView() noexcept {
	if (!IsWindow()) {
		try {
			view_ = CreateView(*m_tabview);
			if (view_ == nullptr) return;
			m_tabview->AddView(view_);
		} catch (...) {}
	} else {	
		m_tabview->ActivateTab(view_);
	}
	if (view_) view_->SetFocus();
}

void CTreeItemAbstract::CloseView() {
	if (view_ == nullptr) return;
	m_tabview->RemoveTab(view_);
}

void CTreeItemAbstract::UpdateTreeLabel() {
	m_treeview.SetItemText(m_hTreeItem, wide(GetName()).c_str());
	if (IsWindow()) {
		std::string title;
		GetViewTitle(title);
		m_tabview->UpdateTitle(view_, title);
		m_tabview->Invalidate();
	}
	auto parent = GetParent();
	if (parent && parent->IsWindow())
		::SendMessage(*parent->GetWindow(), WM_UPDATE_MYLIST, 0, 0);
}

void CTreeItemAbstract::UpdateListView() {
	auto item = this;
	do {
		if (item->IsWindow()) SendMessage(*item->GetWindow(), WM_UPDATE_MYLIST, 0, 0);
	} while ((item = item->GetParent()) != NULL);
}

bool CTreeItemAbstract::ChangeName(const std::string& name) {
	if (name_ == name) return false;
	auto parent = GetParent();
	if (parent) {
		if (parent->FindName(name)) {
			MessageBoxA(g_hMainWnd, 
			("Cannot rename the item \"" + name_ + "\"\r\n"
			 "There is already an item with same name in this category.").c_str(), "npsystem", MB_ICONINFORMATION);
			return false;
		}
		name_ = name;
		return true;
	}
	return false;
}

COnlineTreeItem* CTreeItemAbstract::CreateOnlineFromThis(std::vector<std::unique_ptr<COnlineTreeItem>>& nodes, CTreeViewCtrl tree, HTREEITEM hParent) {
	nodes.push_back(std::move(
		std::make_unique<COnlineTreeItem>(GetName(), m_hIcon)
	));
	nodes.back()->Insert(tree, hParent);
	online_item_ = nodes.back().get();
	return nodes.back().get();
}

COnlineTreeItem* CContainer::CreateOnlineFromThis(std::vector<std::unique_ptr<COnlineTreeItem>>& nodes, CTreeViewCtrl tree, HTREEITEM hParent) {
	nodes.push_back(std::move(
		std::make_unique<COnlineTreeItem>(GetName(), m_hIcon, this)
	));
	nodes.back()->Insert(tree, hParent);
	online_item_ = nodes.back().get();
	return nodes.back().get();
}

int CContainer::MoveBranchImpl(CTreeItemAbstract* branch) {
	if (this == branch->GetParent()) {
		MessageBoxA(NULL, ("Failed to drop \"" + branch->GetName() + "\" onto \"" + GetName() + "\"").c_str(), "", MB_ICONERROR);
		return 0;
	}

	CContainer* parent = this;

	while (parent = parent->GetParent()) {
		if (parent == branch) {
			MessageBoxA(NULL, ("Failed to drop \"" + branch->GetName() + "\" onto \"" + GetName() + "\"").c_str(), "", MB_ICONERROR);
			return 0;
		}
	}

	if (HasItemWithName(branch->GetName()) == true) {
		MessageBoxA(NULL, ("\"" + branch->GetName() + "\" already exists in this category").c_str(), "", MB_ICONERROR);
		return 0;
	}

	Iter::PostorderIterator<CTreeItemAbstract*> post_it(branch);
	for (; !post_it.IsDone(); post_it.Next()) {
		(*post_it)->Detach();
	}

	auto old_parent = branch->GetParent();
	if (old_parent) old_parent->EraseItem(branch);

	branch->Detach();
	AddItem(branch);

	m_treeview.Expand(m_hTreeItem, TVE_EXPAND);

	return 1;
}
