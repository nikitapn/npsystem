// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "stuff.h"

#include <npsys/header.h>
#include <npsys/variable.h>
#include <npsys/other/uploadable.h>
#include <avr_firmware/twi.h>
#include <npdb/observable.h>

class CDlg_Module;

namespace npsys {

class CI2CModuleSegment;

class CI2CModuleSegmentValue 
	: public odb::systree_item_observable<CI2CModuleSegmentValue>
	, public odb::id_support {
	friend CI2CModuleSegment;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<odb::systree_item<CI2CModuleSegmentValue>>(*this);
		ar & boost::serialization::base_object<odb::id_support>(*this);
		ar & type_;
		ar & addr_;
		ar & assigned_;
		if (assigned_) ar & ass;
	}
public:
	class AssignedData : public IUploadable, public odb::observable_removed {
		friend CI2CModuleSegmentValue;
		friend boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int file_version) {
			ar & var;
			ar & status_;
		}

		io::Status status_;

	public:
		npsys::variable_n var;

		io::Status GetStatus() const noexcept { return status_; }
		void SetStatus(io::Status status) noexcept { status_ = status; }
		std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept { 
			return odb::MakeMemento(*this, status_, var);
		}
		void Release() noexcept;
	};

	CI2CModuleSegmentValue() = default;
	CI2CModuleSegmentValue(const std::string& name, int addr, CI2CModuleSegment* _parent, bool assigned)
		: odb::systree_item_observable<CI2CModuleSegmentValue>(name)
		, odb::id_support(true)
		, addr_(addr)
		, type_(variable::VT_BYTE | variable::VQUALITY)
		, parent(_parent)
		, assigned_(assigned)
	{
		if (assigned_) {
			ass = std::make_unique<AssignedData>();
			this->ass->status_ = io::Status::assigned;
		}
	}
	CI2CModuleSegmentValue(const CI2CModuleSegmentValue& other, CI2CModuleSegment* parent) 
		: odb::systree_item_observable<CI2CModuleSegmentValue>(other.name_)
		, odb::id_support(true) {
		this->addr_ = other.addr_;
		this->type_ = other.type_;
		this->parent = parent;
		this->assigned_ = true;
		ass = std::make_unique<AssignedData>();
		this->ass->status_ = io::Status::assigned;
	}
	uint16_t GetAddress() const noexcept { return addr_; }
	int GetType() const noexcept { return type_; }
	int GetSize() const noexcept { return variable::GetSize(type_) + variable::IsQuality(type_); }
	void SetType(int type) noexcept;
	//
	CI2CModuleSegment* parent;
	std::unique_ptr<AssignedData> ass;
private:
	void SetAddress(int addr) noexcept;
	
	int type_;
	uint16_t addr_;
	bool assigned_;
};

using i2c_seg_val_l = std::vector<std::unique_ptr<CI2CModuleSegmentValue>>;

class CI2CModuleSegment 
	: public odb::systree_item_observable<CI2CModuleSegment>
	, public odb::id_support {
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int /*file_version*/) const {}
	template<class Archive>
	void load(Archive & ar, const unsigned int /*file_version*/) {
		for (auto& val : values) {
			val->parent = this;
		}
	}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<odb::systree_item<CI2CModuleSegment>>(*this);
		ar & boost::serialization::base_object<odb::id_support>(*this);
		ar & offset_;
		ar & seg_type_;
		ar & values;
		ar & parent_module;
		boost::serialization::split_member(ar, *this, file_version);
	}
public:
	using list_t = i2c_seg_val_l;

	CI2CModuleSegment() = default;
	explicit CI2CModuleSegment(bool new_id) 
		: odb::id_support(true) {}
	explicit CI2CModuleSegment(const std::string& name)
		: odb::systree_item_observable<CI2CModuleSegment>(name)
		, odb::id_support(true)
		, seg_type_(io::SegmentType::READ)
		, offset_(0) {}
	
	CI2CModuleSegment(const CI2CModuleSegment&) = delete;
	CI2CModuleSegment& operator=(const CI2CModuleSegment&) = delete;

	i2c_seg_val_l values;

	int GetLenght() const;
	io::SegmentType GetType() const noexcept { return seg_type_; }
	void SetType(io::SegmentType type) noexcept { seg_type_ = type; }
	uint16_t GetOffset() const noexcept { return offset_; }
	void SetOffset(uint16_t offset);
	CI2CModuleSegmentValue* CreateNewSegmentValue(const std::string& name, bool assigned) noexcept;
	void RecalcFrom(CI2CModuleSegmentValue* from) noexcept;
	void RecalcAll() noexcept;
	std::unique_ptr<CI2CModuleSegment> clone() const noexcept {
		auto copy = std::make_unique<CI2CModuleSegment>(true);
		copy->name_ = name_;
		copy->offset_ = offset_;
		copy->seg_type_ = seg_type_;
		for (auto const& val : values) {
			copy->values.push_back(std::move(
				std::unique_ptr<CI2CModuleSegmentValue>(new CI2CModuleSegmentValue(*val.get(), copy.get()))
			));
		}
		return copy;
	}
	
	odb::weak_node<i2c_assigned_module_n> parent_module;
protected:
	void Recalc(i2c_seg_val_l::iterator first, i2c_seg_val_l::iterator last);
	uint16_t offset_;
	io::SegmentType seg_type_;
};

class CI2CModule
	: public odb::common_interface<odb::no_interface>
	, public odb::systree_item_observable<CI2CModule>
	, public odb::self_node_member<npsys::i2c_module_n>
	, public odb::modified_flag {
	friend CDlg_Module;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & boost::serialization::base_object<odb::systree_item<CI2CModule>>(*this);
		ar & slave_addr_;
		ar & addr_size_;
		ar & childs;
	}
public:
	using list_t = odb::object_list<CI2CModuleSegment>;

	CI2CModule() = default;
	CI2CModule(const std::string& name) 
		: odb::systree_item_observable<CI2CModule>(name)
		, slave_addr_(0x80)
		, addr_size_(0) {
		childs.create();
	}
	uint8_t slave_addr() const noexcept { return slave_addr_; }
	int addr_size() const noexcept { return addr_size_; }
	auto CreateSegment(const std::string& name) {
		return childs.emplace_back(name);
	}
	odb::object_list<CI2CModuleSegment> childs;
protected:
	uint8_t slave_addr_;
	int addr_size_;
};

class CAssignedI2CModule : public CI2CModule
{
	friend CDlg_Module;
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & boost::serialization::base_object<CI2CModule>(*this);
		ar & ctrl;
		ar & lrr;
	}

public:
	using list_t = CI2CModule::list_t;

	CAssignedI2CModule() = default;
	CAssignedI2CModule(const CI2CModule& from, controller_n& _ctrl)
		: ctrl(_ctrl) {
		ATLASSUME(from.childs.loaded());
		name_ = from.get_name();
		slave_addr_ = from.slave_addr();
		addr_size_ = from.addr_size();
		childs = std::move(from.childs.clone());
	}

	auto CreateSegment(const std::string& name) {
		auto segment = childs.emplace_back(name);
		segment->parent_module = self_node;
		return segment;
	}

	CI2CModuleSegmentValue* GetSegmentValue(oid_t id) noexcept {
		if (!childs.fetch()) return nullptr;
		for (auto& segment : childs) {
			for (auto& value : segment->values) {
				if (value->GetId() == id) return value.get();
			}
		}
		return nullptr;
	}

	std::vector<twi_req_t> lrr; // loaded relevant requests
	odb::weak_node<controller_n> ctrl;
};

} // namespace npsys