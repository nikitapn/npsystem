// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "tr_system.h"
#include "tr_network.h"
#include "tr_alg.h"
#include "tr_block.h"
#include "algext.h"
#include "view_algorithm.h"
#include "dlgalgorithmproperties.h"
#include "dlgstring.h"
#include "dlgsimpleinput.h"
#include "assignedalgorithm.h"
#include "network_ext.h"

#include "tr_module.h"
#include "tr_controller.h"

#include <npsys/algcat.h>
#include <npsys/algsubcat.h>
#include <npsys/web.h>
#include <npsys/strings.h>


// CTreeAlgorithm
CTreeAlgorithm::CTreeAlgorithm(npsys::algorithm_n& n, npsys::algorithm_l& l)
	: LazyItemListContainer(n, l) {
	SetIcon(AlphaIcon::Algorithm);
	n_->tree_algorithm = this;
}

void CTreeAlgorithm::ConstructMenu(CMenu* menu) noexcept {
	UINT assigned = (n_->GetStatus() >= npsys::CAlgorithm::status_assigned) ? 0xFFFFFFFF : 0;
	UINT loaded = (n_->GetStatus() >= npsys::CAlgorithm::status_loaded) ? 0xFFFFFFFF : 0;

	menu->AppendMenu(MF_STRING | (MF_DISABLED & ~assigned), _ID_UPLOAD_ALGORITHM_, L"Upload to controller");
	menu->AppendMenu(MF_STRING | (MF_DISABLED & ~loaded), _ID_UNLOAD_ALGORITHM_, L"Unload");
	menu->AppendMenu(MF_STRING | (MF_DISABLED & assigned), ID_ITEM_DELETE, L"Delete");
#ifdef DEBUG
	menu->AppendMenu(MF_SEPARATOR);
	menu->AppendMenu(MF_STRING, ID_ALGORITHM_SHOW_BLOCKS, L"Show blocks");
#endif // DEBUG
	menu->AppendMenu(MF_SEPARATOR);
	menu->AppendMenu(MF_STRING, ID_ITEM_PROPERTIES, L"Properties");
}

INT_PTR CTreeAlgorithm::ShowProperties() {
	CDlg_AlgorithmProperties dlg(n_);
	return dlg.DoModal(g_hMainWnd);
}

bool CTreeAlgorithm::ChangeName(const std::string& name) {
	if (CTreeItemAbstract::ChangeName(name) == false) return false;
	n_->set_name(name);
	if (n_->GetStatus() >= npsys::CAlgorithm::status_assigned) {
		auto assigned = n_->assigned_alg.fetch();
		ASSERT(assigned.loaded());
		if (assigned->item) {
			assigned->item->ForceSetName(name);
			assigned->item->UpdateTreeLabel();
		}
	}
	n_.store();
	return true;
}

void CTreeAlgorithm::HandleRequest(REQUEST* req) noexcept {
	switch (req->ID_MENU) {
	case _ID_UPLOAD_ALGORITHM_:
		n_->UploadToDevice(true);
		break;
	case _ID_UNLOAD_ALGORITHM_:
		n_->UnloadFromDevice();
		break;
#ifdef DEBUG
	case ID_ALGORITHM_SHOW_BLOCKS:
		n_->ShowBlocks();
		break;
#endif // DEBUG
	default:
		__super::HandleRequest(req);
	}
}

CItemView* CTreeAlgorithm::CreateView(CMyTabView& tabview) {
	return CreateViewForItem<CAlgorithmView>(this, tabview);
};

void CTreeAlgorithm::LoadChildsImpl() noexcept {
	n_->fbd_blocks.fetch_all_nodes();
	for (auto& i : n_->fbd_blocks) {
		insert(new CTreeBlock(i));
	}
}

int CTreeAlgorithm::Move(CTreeItemAbstract* item, int action) {
	int res = item->Move(this, action);
	return res;
}

void CTreeAlgorithm::PostDelete(CContainer* parent) noexcept {
	n_->DeletePermanent();
	__super::PostDelete(parent);
}

bool CTreeAlgorithm::IsRemovable() {
	bool removable = (n_->GetStatus() == npsys::CAlgorithm::status_not_assigned);
	if (!removable && n_->assigned_dev.is_invalid_id() != false) {
		return true;
	}
	if (!removable) {
		MessageBoxA(g_hMainWnd, ("Cannot remove " + GetName() + "\r\n"
			"The algorithm is assigned").c_str(), "npsystem", MB_ICONEXCLAMATION);
	}
	return removable;
}

// CTreeAlgorithmCat
class CTreeAlgorithmCat : public LazyItemListContainer<CTreeAlgorithmCat,
	std::vector, CTreeAlgorithm, DatabaseElement<npsys::algorithm_category_n, npsys::algorithm_category_l>,
	TableChilds>
{
public:
	CTreeAlgorithmCat(npsys::algorithm_category_n& n, npsys::algorithm_category_l& l)
		: LazyItemListContainer(n, l) {
		SetIcon(AlphaIcon::Container);
	}
	virtual void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_CREATE_ITEM, L"New Algorithm");
		DefaultMenu(menu);
	}

	virtual void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_CREATE_ITEM:
			CreateNewItem(n_->algs, GetNextName("ALGORITHM_"), n_);
			break;
		default:
			__super::HandleRequest(req);
		};
	}
	auto& childs_node_list() { return n_->algs; }
protected:
	virtual bool IsRemovable() {
		m_treeview.Expand(m_hTreeItem, TVE_EXPAND);
		for (auto alg : m_items) {
			if (alg->n_->GetStatus() > npsys::CAlgorithm::status_not_assigned) {
				MessageBoxA(NULL, ("Cannot remove category \"" + GetName() + "\" algorithm \""
					+ alg->GetName() + "\" is assigned").c_str(), "", MB_ICONWARNING);
				return false;
			}
		}
		return true;
	}
	virtual void PostDelete(CContainer* parent) noexcept {
		n_->algs.fetch_all_nodes();
		for (auto& alg : n_->algs) {
			alg->DeletePermanent();
			alg.remove();
		}
		n_->algs.remove();
		__super::PostDelete(parent);
	};
};

// CTreeAlgorithms
class CTreeAlgorithms
	: public LazyItemListContainer<CTreeAlgorithms, std::vector, CTreeAlgorithmCat, FixedElement>
{
public:
	CTreeAlgorithms() {
		SetIcon(AlphaIcon::Container);
		name_ = "ALGORITHMS";
		if (!algorithms_.fetch()) {
			algorithms_.create();
		}
	}
	virtual void HandleRequest(REQUEST* req) noexcept final {
		switch (req->ID_MENU) {
		case ID_CREATE_ITEM:
			CreateNewItem(algorithms_->alg_cats, GetNextName("ALGORITHM_CATEGORY_"));
			break;
		default:
			__super::HandleRequest(req);
		};
	}
	virtual void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_CREATE_ITEM, L"New Algorithm Category");
	}
	virtual oid_t GetId() const noexcept final {
		return npsys::ALGORITHMS_ID;
	}
	virtual bool IsRemovable() { return false; }

	auto& childs_node_list() { return algorithms_->alg_cats; }
protected:
	npsys::categories_n algorithms_;
};

// CTreeWebItem


class CTreeWebItem : public CTemplateItem<
	CTreeWebItem,
	CTreeItemAbstract,
	DatabaseElement<npsys::web_item_n, npsys::web_item_l>> {
public:
	CTreeWebItem(npsys::web_item_n& n, npsys::web_item_l& parent)
		: CTemplateItem(n, parent) {
		SetIcon(AlphaIcon::Library);
		UnsetUpperCase();
	}

	virtual void ConstructMenu(CMenu* menu) noexcept {
		DefaultMenu(menu);
	}

	virtual INT_PTR ShowProperties() {
		std::string path = n_->slot.fetch() ? n_->slot->path() : "<INVALID PATH>";
		npsys::fbd_slot_n tmp;
		if (mctrl::show_dlg_simple_input(
			n_->get_name().c_str(),
			mctrl::make_text_edit("Name: ", false, *n_.get()),
			mctrl::make_text_edit("Description:", false, n_->description),
			mctrl::make_text_edit("Path:", true, path, [&tmp](std::string& path) {
				tmp = npsys::CFBDSlot::get_by_path(path);
				})
		 ) == IDOK) {
			n_->slot = tmp;
			n_.store();
			this->UpdateTreeLabel(n_->get_name());
			this->GetParent()->UpdateListView();
		}
		return 0;
	}

	virtual void HandleRequest(REQUEST* req) noexcept {
		//switch (req->ID_MENU) {

		//default:
			__super::HandleRequest(req);
		//};
	}


};


// CTreeWebCat

class CTreeWebCat
	: public LazyItemListContainer<CTreeWebCat, std::vector, CTreeWebItem, DatabaseElement<npsys::web_cat_n, npsys::web_l>> {
public:
	CTreeWebCat(npsys::web_cat_n& n, npsys::web_l& parent)
		: LazyItemListContainer(n, parent) {
		SetIcon(AlphaIcon::Library);
		UnsetUpperCase();
	}

	virtual CItemView* CreateView(CMyTabView& tabview) final {
		return CreateViewForItem<CListControlView>(this, tabview);
	};

	virtual CViewAdapter* GetViewAdapter() {
		auto adapter = new CViewAdapterTempData<std::string>(this);
		adapter->m_columns.emplace_back(L"Name", "%s", 30);
		adapter->m_columns.emplace_back(L"Desription", "%s", 35);
		adapter->m_columns.emplace_back(L"Path", "%s", 35);
		for (auto item : *this) {
			auto& n = item->node();
			char const* path_ptr = "<INVALID PATH>";
			if (n->slot.fetch()) {
				adapter->temp_data.push_back(std::make_unique<std::string>(n->slot->path()));
				path_ptr = adapter->temp_data.back()->c_str();
			}
			adapter->m_data.emplace_back(
				(LPARAM)item,
				(LPARAM)n->get_name().c_str(),
				(LPARAM)n->description.c_str(),
				(LPARAM)path_ptr
			);
		}

		return adapter;
	}

	virtual void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_CREATE_ITEM, L"New Item");
	}

	virtual void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_CREATE_ITEM:
			CreateNewItem(n_->items, GetNextName("WEB_ITEM_"));
			break;
		default:
			__super::HandleRequest(req);
		};
	}
	auto& childs_node_list() { return n_->items; }
};

// CTreeWebRoot

class CTreeWebRoot :
	public LazyItemListContainer<CTreeWebRoot, std::vector, CTreeWebCat, FixedElement> {
	npsys::web_l lst_;
public:
	CTreeWebRoot() {
		SetIcon(AlphaIcon::Network);
		name_ = "WEB";
	}
	virtual oid_t GetId() const noexcept final { return npsys::WEB; }
	virtual void HandleRequest(REQUEST* req) noexcept final {
		switch (req->ID_MENU) {
		case ID_CREATE_ITEM:
			CreateNewItem(lst_, GetNextName("CATEGORY_"));
			break;
		default:
			__super::HandleRequest(req);
		};
	}
	virtual void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_CREATE_ITEM, L"New Category");
	}
	virtual bool IsRemovable() { return false; }
	auto& childs_node_list() { return lst_; }
};

// String

class String : public InteractiveItem {
	CTreeItemAbstract* parent_item_;
	npsys::strings_n& strings_;
	size_t number_;
public:
	String(CTreeItemAbstract* parent_item, npsys::strings_n& strings, size_t number)
		: parent_item_(parent_item)
		, strings_(strings)
		, number_(number) {}

	virtual void ConstructMenu(CMenu* menu) noexcept {
		DefaultMenu(menu);
	}

	virtual void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_ITEM_DELETE:
			if (MessageBoxA(g_hMainWnd, ("Delete \"" + *strings_->get(number_) + "\"?").c_str(), "", MB_OKCANCEL) == IDOK) {
				strings_->remove(number_);
				strings_.store();
				parent_item_->UpdateListView(); // this become invalid after this call
			}
			break;
		case ID_ITEM_PROPERTIES: {
			CDlg_String dlg(number_, strings_);
			if (dlg.DoModal() == IDOK) {
				strings_.store();
				parent_item_->UpdateListView(); // this become invalid after this call
			}
			break;
		}
		default:
			std::cout << "Unknown request: " << req->ID_MENU << std::endl;
			break;
		};
	}
};

// CTreeStrings

class CTreeStrings : public CTemplateItem<
	CTreeStrings,
	CTreeItemAbstract,
	DatabaseElement<npsys::strings_n, npsys::strings_l>> {
public:
	CTreeStrings(npsys::strings_n& n, npsys::strings_l& parent)
		: CTemplateItem(n, parent) {
		SetIcon(AlphaIcon::Library);
	}

	virtual CItemView* CreateView(CMyTabView& tabview) final {
		return CreateViewForItem<CListControlView>(this, tabview);
	};

	virtual CViewAdapter* GetViewAdapter() {
		auto adapter = new CViewAdapterTempData<String>(this);
		adapter->m_columns.emplace_back(L"Name", "%s", 60);
		adapter->m_columns.emplace_back(L"Value", "%d", 40);
		for (const auto& item : *n_.get()) {
			adapter->temp_data.push_back(std::make_unique<String>(this, n_, item.first));
			adapter->m_data.emplace_back(
				(LPARAM)static_cast<InteractiveItem*>(adapter->temp_data.back().get()),
				(LPARAM)item.second.c_str(),
				(LPARAM)item.first
			);
		}
		return adapter;
	}

	virtual void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_CREATE_ITEM, L"New String");
	}

	virtual void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_CREATE_ITEM:
		{
			size_t max = 0;
			for (const auto& item : *n_.get()) {
				if (item.first > max) max = item.first;
			}
			n_->set(max + 1, "Example String");
			UpdateListView();
			n_.store();
			break;
		}
		default:
			__super::HandleRequest(req);
		};
	}
};

// CTreeStrings
class CTreeStringsCat : public LazyItemListContainer<CTreeStringsCat, std::vector, CTreeStrings, FixedElement> {
public:
	CTreeStringsCat() {
		SetIcon(AlphaIcon::Container);
		name_ = "Strings";
	}
	virtual void HandleRequest(REQUEST* req) noexcept final {
		switch (req->ID_MENU) {
		case ID_CREATE_ITEM:
			CreateNewItem(strings_, GetNextName("STRINGS_COLLECTION_"));
			break;
		default:
			__super::HandleRequest(req);
		};
	}
	virtual void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_CREATE_ITEM, L"New Strings Collection");
	}
	virtual oid_t GetId() const noexcept final {
		return npsys::STRINGS;
	}
	virtual bool IsRemovable() { return false; }
	auto& childs_node_list() { return strings_; }
protected:
	npsys::strings_l strings_;
};


// CTreeSystem
CTreeSystem::CTreeSystem(npsys::system_n& n)
	: ItemListContainer(n) {
	LoadName();
	LoadChilds();
	SetIcon(AlphaIcon::Root);
	NPSystemCompileLibraries();
}

CTreeSystem::~CTreeSystem() {
	Iter::PostorderIterator<CTreeItemAbstract*> it(this);
	for (; !it.IsDone(); it.Next()) {
		delete (*it);
	}
}

void CTreeSystem::LoadChildsImpl() noexcept {
	mp11::mp_for_each<mp11::mp_iota_c<mp11::mp_size<items_t>::value>>
		([this](auto I) { insert(new mp11::mp_at_c<items_t, I>); });
}

void CTreeSystem::CalcStatusAll() {
	auto network = Get<CTreeNetwork>();
	Iter::IteratorPtr<CTreeItemAbstract*> it(network);
	for (; !it->IsDone(); it->Next()) (*it)->CalcAndUpdateHardwareStatus();
}