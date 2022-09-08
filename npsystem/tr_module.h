// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "tr_item.h"
#include "listcontrolview.h"
#include "module.h"
#include "dlgmodule.h"
#include "dlgsegment.h"
#include "dlgsegmentvalue.h"

class CTreeAssignedSegmentValue;

template<class MostDerrived, class SegmentType, class NodeType, class NodeList >
class CTreeModuleT
	: public LazyItemListContainer<MostDerrived,
	std::vector, SegmentType, DatabaseElement<NodeType, NodeList>, DataChilds>
{
	using base = LazyItemListContainer<MostDerrived, std::vector, SegmentType, DatabaseElement<NodeType, NodeList>, DataChilds>;
public:
	CTreeModuleT(NodeType& n, NodeList& l)
		: base(n, l) {}

	virtual void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_CREATE_ITEM, L"New Segment");
		menu->AppendMenu(MF_SEPARATOR);
		base::DefaultMenu(menu);
	}
	
	virtual INT_PTR ShowProperties() {
		CDlg_Module dlg(*base::n_.get(), base::GetName());
		auto result = dlg.DoModal();
		if (result == IDOK) {
			this->StoreItem();
		}
		return result;
	}
	
	void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_CREATE_ITEM:
		{
			this->m_treeview.Expand(base::m_hTreeItem, TVE_EXPAND);
			ASSERT_FETCH(base::n_->childs);
			this->AddItem(new SegmentType(
				this->n_->CreateSegment(this->GetNextName("DS_")),
				this->n_->childs
			));
			this->UpdateListView();
			this->StoreItem();
			break;
		}
		default:
			__super::HandleRequest(req);
		};
	}
	
	virtual CItemView* CreateView(CMyTabView& tabview) final {
		return CreateViewForItem<CListControlView>(this, tabview);
	};
	virtual CViewAdapter* GetViewAdapter() {
		auto adapter = new CViewAdapter(this);
		adapter->m_columns.emplace_back(L"Segment", "%s", 40);
		adapter->m_columns.emplace_back(L"Offset", "0x%.4x", 30);
		adapter->m_columns.emplace_back(L"Length", "%d", 30);
		for (auto item : base::m_items) {
			adapter->m_data.emplace_back(
				(LPARAM)item,
				(LPARAM)item->GetName().c_str(),
				item->data()->GetOffset(),
				item->data()->GetLenght()
			);
		}
		return adapter;
	}
	virtual void PostDelete(CContainer* parent) noexcept {
		this->n_->childs.remove();
		__super::PostDelete(parent);
	};
};

template<typename T, bool Assigned>
class CTreeSegmentValueT : public T {
	using base = T;/* CTemplateItem<CTreeItemAbstract,
		CategoryElement<npsys::CI2CModuleSegmentValue, npsys::CI2CModuleSegment::list_t>>;*/
	friend class CDlg_SegmentValue;
	friend class CAssignedSegmentValue;
public:
	constexpr static auto IsAssigned = Assigned;

	CTreeSegmentValueT(npsys::CI2CModuleSegmentValue* data, 
		npsys::CI2CModuleSegment::list_t& list, npsys::CI2CModuleSegment& segment)
		: base(data, list)
		, segment_(segment) {
		this->SetIcon(NPSystemIcon::Fun);
	}
	virtual void ConstructMenu(CMenu* menu) noexcept {
		base::DefaultMenu(menu);
	}
	INT_PTR ShowProperties() {
		CDlg_SegmentValue dlg(*T::data_, *this);
		auto const result = dlg.DoModal();
		if (result == IDOK) {
			this->data_->parent->RecalcFrom(this->data_);
		}
		return result;
	}
protected:
	npsys::CI2CModuleSegment& segment_;
};

template<typename T>
class CTreeSegmentT : public T {
	using base = T;
	using ChildType = typename T::ChildType;
public:
	CTreeSegmentT(npsys::CI2CModuleSegment* data, npsys::CI2CModule::list_t& list)
		: base(data, list) {
		this->SetIcon(NPSystemIcon::Fun);
		this->LoadChilds();
	}
	void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_CREATE_ITEM, L"New Value");
		menu->AppendMenu(MF_SEPARATOR);
		base::DefaultMenu(menu);
	}
	INT_PTR ShowProperties() {
		CDlg_Segment dlg(*this->data_, base::GetName());
		return dlg.DoModal();
	}

	constexpr std::string_view ToStringType(int type) {
		using namespace npsys::nptype;
		using namespace std::string_view_literals;

		switch (npsys::variable::GetClearType(type)) {
			case NPT_U8:	return "u8"sv;
			case NPT_I8:	return "i8"sv;
			case NPT_U16: return "u16"sv;
			case NPT_I16: return "i16"sv;
			case NPT_U32: return "u32"sv;
			case NPT_I32: return "i32"sv;
			case NPT_F32: return "f32"sv;
			default: return "unknown type"sv;
		}
	}

	virtual CItemView* CreateView(CMyTabView& tabview) final {
		return CreateViewForItem<CListControlView>(this, tabview);
	};

	virtual CViewAdapter* GetViewAdapter() {
		CViewAdapter* adapter = new CViewAdapter(this);
		adapter->m_columns.emplace_back(L"Value", "%s", 40);
		adapter->m_columns.emplace_back(L"Type", "%s", 15);
		adapter->m_columns.emplace_back(L"Offset", "0x%.4x", 15);
		adapter->m_columns.emplace_back(L"Size", "%d", 15);
		adapter->m_columns.emplace_back(L"Status", "%d", 15);
		for (auto item : this->m_items) {
			auto const value = item->data();
			adapter->m_data.emplace_back(
				(LPARAM)static_cast<InteractiveItem*>(item),
				(LPARAM)item->GetName().c_str(),
				(LPARAM)ToStringType(value->GetType()).data(),
				value->GetAddress(),
				value->GetSize(),
				npsys::variable::IsQuality(value->GetType())
			);
		}
		return adapter;
	}

	void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_CREATE_ITEM:
			this->m_treeview.Expand(this->m_hTreeItem, TVE_EXPAND);
			this->AddItem(new ChildType(
				this->data_->CreateNewSegmentValue(this->GetNextName("VALUE_"), ChildType::IsAssigned),
				this->data_->values, *this->data_)
			);
			this->UpdateListView();
			this->StoreItem();
			break;
		default:
			__super::HandleRequest(req);
		};
	}
protected:
	virtual void LoadChildsImpl() noexcept {
		for (auto& val : this->data_->values) {
			this->insert(new ChildType(val.get(), this->data_->values, *this->data_));
		}
	}
};

class CTreeSegmentValue : public CTreeSegmentValueT<CTemplateItem<CTreeSegmentValue, CTreeItemAbstract,
	CategoryElement<npsys::CI2CModuleSegmentValue, npsys::CI2CModuleSegment::list_t>>, false>  {
public:
	CTreeSegmentValue(npsys::CI2CModuleSegmentValue* data,
		npsys::CI2CModuleSegment::list_t& list, npsys::CI2CModuleSegment& segment)
		: CTreeSegmentValueT(data, list, segment) {}
};

class CTreeSegment : public CTreeSegmentT<ItemListContainer<CTreeSegment, std::vector, CTreeSegmentValue,
	CategoryElement<npsys::CI2CModuleSegment, npsys::CI2CModule::list_t>>> {
public:
	CTreeSegment(npsys::CI2CModuleSegment* data, npsys::CI2CModule::list_t& list)
		: CTreeSegmentT(data, list) {}
};

class CTreeI2CModule : public CTreeModuleT<CTreeI2CModule, CTreeSegment, npsys::i2c_module_n, npsys::i2c_module_l> {
public:
	CTreeI2CModule(npsys::i2c_module_n& n, npsys::i2c_module_l& l)
		: CTreeModuleT(n, l) {
		SetIcon(NPSystemIcon::Module);
	}
	virtual int Move(CTreeItemAbstract* item, int action) {
		int res = item->Move(this, action);
		return res;
	}
};

class CI2CModules 
	: public LazyItemListContainer<CI2CModules, std::vector, CTreeI2CModule, FixedElement> {
public:
	CI2CModules() {
		SetIcon(NPSystemIcon::I2C);
		ForceSetName("I2C");
	}
	virtual oid_t GetId() const noexcept final {
		return npsys::I2C_MODULES_ID;
	}
	virtual bool IsRemovable() { return false; }
	void ConstructMenu(CMenu* menu) noexcept {
		menu->AppendMenu(MF_STRING, ID_CREATE_ITEM, L"New Module");
	}
	void HandleRequest(REQUEST* req) noexcept {
		switch (req->ID_MENU) {
		case ID_CREATE_ITEM:
		{
			odb::Batch batch;
			CreateNewItem<npsys::CI2CModule>(list_, GetNextName("MODULE_"));
			UpdateListView();
			batch.exec();
			break;
		}
		default:
			__super::HandleRequest(req);
		};
	}
protected:
	virtual void LoadChildsImpl() noexcept {
		if (list_.fetch_all_nodes()) {
			for (auto& mod : list_) {
				insert(new CTreeI2CModule(mod, list_));
			}
		}
	}
	npsys::i2c_module_l list_;
};

class CTreeModules 
	: public LazyItemListContainer<CTreeModules, std::vector, CTreeItemAbstract, FixedElement>
{
public:
	CTreeModules() {
		SetIcon(NPSystemIcon::Modules);
		ForceSetName("MODULES");
	}
	virtual oid_t GetId() const noexcept final {
		return npsys::MODULES_ID;
	}
	virtual bool IsRemovable() { return false; }
protected:
	virtual void LoadChildsImpl() noexcept {
		insert(new CI2CModules());
	}
};
