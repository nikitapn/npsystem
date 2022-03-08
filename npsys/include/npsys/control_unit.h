// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "header.h"
#include "access.h"
#include <npdb/memento.h>
#include <npsys/other/uploadable.h>

namespace npsys {

struct CAlgorithmViewPosition 
	: public odb::common_interface<odb::no_interface>
	, public odb::modified_flag {

	float x_offset;
	float y_offset;
	float scale;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & x_offset;
		ar & y_offset;
		ar & scale;
	}

	void DefaultViewPosition() noexcept {
		x_offset = 0.0f;
		y_offset = 0.0f;
		scale = 1.5f;
	}

	void SavePosition(float _x_offset, float _y_offset, float _scale) noexcept {
		if (std::abs(x_offset - _x_offset) > 0.001f ||
			std::abs(y_offset - _y_offset) > 0.001f ||
			std::abs(scale - _scale) > 0.001f) {
			set_modified(true);
		}
		x_offset = _x_offset;
		y_offset = _y_offset;
		scale = _scale;
	}
};

using algorithm_view_position_n = odb::shared_node<CAlgorithmViewPosition>;

class CControlUnit 
	: public odb::common_interface<CControlUnit>
	, public odb::systree_item<CControlUnit>
	, public odb::self_node_member<npsys::control_unit_n>
	, public odb::modified_flag 
	, public IUploadable {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & name_;
		ar & cat;
		ar & assigned_dev;
		ar & assigned_alg;
		ar & npsys::access::rw(scan_period);
		ar & view_position;
		ar & status_;
	}
public:
	static constexpr int _BOOST_CLASS_VERSION = 1;
	
	enum class Language {
		FBD, 
		SFC
	};

	enum class Status {
		status_not_assigned,
		status_assigned,
		status_loaded,
		status_not_actual
	};
	
	odb::weak_node<algorithm_category_n> cat;
	odb::weak_node<device_n> assigned_dev;
	odb::weak_node<assigned_algorithm_n> assigned_alg;
	const uint16_t scan_period = 500;
	algorithm_view_position_n view_position;
public:
	Status GetStatus() const noexcept { return status_; }
	void SetStatus(Status status) noexcept { status_ = status; }
	void SetScanPeriodMs(uint16_t scan_period);
	
	virtual Language GetLanguageType() const noexcept = 0;
	virtual std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept final {
		return odb::MakeMemento(*this, status_);
	}
#ifdef _CONFIGURATOR_
protected:
	bool online_ = false;
	CWindow window_;
public:
	CTreeControlUnit* tree_algorithm = nullptr;
	ActionList actions;

	void CloseView();
	bool IsOnline() const noexcept { return online_; }
	void SetWindow(CWindow window) noexcept { window_ = window; }
	void OnlineRestart();

	virtual CItemView* CreateView(CTreeControlUnit* tree, CMyTabView& tabview) = 0;

	virtual void DeletePermanent() noexcept = 0;
	virtual void UploadToDevice(bool mb_confirm = false) noexcept = 0;
	virtual void UnloadFromDevice() noexcept = 0;
	virtual void Unload() = 0;
	virtual void HardwareSpecific_Clear() = 0;
	virtual void Save(bool after_uploading = false) noexcept = 0;
	virtual INT_PTR ShowProperties(control_unit_n& self) = 0;
	virtual NPSystemIcon GetIcon() const noexcept = 0;
	virtual void StartOnline() = 0;
	virtual void StopOnline() = 0;
	virtual CBlockComposition* GetBlocks() = 0;
	virtual void DrawOnlineValues(CGraphics* pGraphics) = 0;
	virtual bool IsUndoRedo() = 0;
	virtual void UpdateData(std::vector<nps::server_value>* upd) = 0;
#endif
private:
	Status status_ = Status::status_not_assigned;
};

#ifdef _CONFIGURATOR_
class CFBDControlUnit 
	: public CControlUnit
	, public IFatDataCallBack {
#else
class CFBDControlUnit 
	: public CControlUnit {
#endif
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CControlUnit>(*this);
		ar & editor_;
		ar & fbd_blocks;
	}
public:
	odb::node_list<fbd_block_n> fbd_blocks;

	virtual Language GetLanguageType() const noexcept { return CControlUnit::Language::FBD; }
protected:
	block_composition_n editor_;
#ifdef _CONFIGURATOR_
	CFindLoadedSlots m_loadedSlots;
	std::map<nps::var_handle, std::vector<CSlot*>> ref_;
	bool saving_in_progress_ = false;
	int	m_running = 0;
protected:
	// CORBA
	virtual size_t AdviseImpl();
	virtual void OnDataChangedImpl(::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> a);
	virtual void OnConnectionLost() noexcept;
public:
	void Translate(CCodeGenerator*);
	void Separate(std::vector<CBlock*>*, CBlock*, char*);
	int CalcOrder(CBlock*, int, char*);
	int TryExecuteNext(CBlock*, int, char*);
	void ClearVariables() noexcept;
	void DetermineTypes(npsys::fbd_control_unit_n& self);
	std::optional<npsys::fbd_slot_n> FindSlot(const std::string& block_name, const std::string& slot_name);
	std::optional<npsys::fbd_slot_n> FindSlot(int slotId);
	void ShowBlocks();
	//

	virtual CItemView* CreateView(CTreeControlUnit* tree, CMyTabView& tabview);
	virtual void DeletePermanent() noexcept;
	virtual void UploadToDevice(bool mb_confirm = false) noexcept;
	virtual void UnloadFromDevice() noexcept;
	virtual void Unload();
	virtual void HardwareSpecific_Clear();
	virtual void Save(bool after_uploading = false) noexcept;
	virtual INT_PTR ShowProperties(control_unit_n& self);
	virtual NPSystemIcon GetIcon() const noexcept;
	virtual void StartOnline();
	virtual void StopOnline();
	virtual CBlockComposition* GetBlocks();
	virtual void DrawOnlineValues(CGraphics* pGraphics);
	virtual bool IsUndoRedo();
	virtual void UpdateData(std::vector<nps::server_value>* upd);
	//						
	CFBDControlUnit(std::string&& name, npsys::algorithm_category_n& cat);
#endif
	CFBDControlUnit() = default;
};

#ifdef _CONFIGURATOR_
class CSFCControlUnit 
	: public CControlUnit
	, public IFatDataCallBack {
#else
class CSFCControlUnit 
	: public CControlUnit {
#endif
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CControlUnit>(*this);
		ar & editor_;
	}
public:
	virtual Language GetLanguageType() const noexcept { return CControlUnit::Language::SFC; }
protected:
	sfc_block_composition_n editor_;
#ifdef _CONFIGURATOR_
protected:
	// CORBA
	virtual size_t AdviseImpl();
	virtual void OnDataChangedImpl(::flat::Span_ref<nps::flat::server_value, nps::flat::server_value_Direct> a);
	virtual void OnConnectionLost() noexcept;
public:
	//

	virtual CItemView* CreateView(CTreeControlUnit* tree, CMyTabView& tabview);
	virtual void DeletePermanent() noexcept;
	virtual void UploadToDevice(bool mb_confirm = false) noexcept;
	virtual void UnloadFromDevice() noexcept;
	virtual void Unload();
	virtual void HardwareSpecific_Clear();
	virtual void Save(bool after_uploading = false) noexcept;
	virtual INT_PTR ShowProperties(control_unit_n& self);
	virtual NPSystemIcon GetIcon() const noexcept;
	virtual void StartOnline();
	virtual void StopOnline();
	virtual CBlockComposition* GetBlocks();
	virtual void DrawOnlineValues(CGraphics* pGraphics);
	virtual bool IsUndoRedo();
	virtual void UpdateData(std::vector<nps::server_value>* upd);
	//						
	CSFCControlUnit(std::string&& name, npsys::algorithm_category_n& cat);
#endif
	CSFCControlUnit() = default;
};



} // namespace npsys

//BOOST_CLASS_VERSION(npsys::CControlUnit, npsys::CControlUnit::_BOOST_CLASS_VERSION);
