// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include <npsys/variable.h>
#include "network_ext.h"
#include "assignedalgorithm.h"
#include <npdb/memento_manager.h>

namespace npsys {

void variable::m_DefaultValue_Clear(nptype::Type tp) noexcept {
	switch (tp) {
	case nptype::NPT_BOOL:
		def_value_._b = static_cast<bool>(0);
		break;
	case nptype::NPT_U8:
		def_value_._u8 = static_cast<u8>(0);
		break;
	case nptype::NPT_I8:
		def_value_._i8 = static_cast<i8>(0);
		break;
	case nptype::NPT_U16:
		def_value_._u16 = static_cast<u16>(0);
		break;
	case nptype::NPT_I16:
		def_value_._i16 = static_cast<i16>(0);
		break;
	case nptype::NPT_U32:
		def_value_._u32 = static_cast<u32>(0);
		break;
	case nptype::NPT_I32:
		def_value_._i32 = static_cast<i32>(0);
		break;
	case nptype::NPT_F32:
		def_value_._f32 = static_cast<f32>(0.0f);
		break;
	default:
		memset(def_value_.data, 0x00, sizeof(def_value_.data));
		break;
	}
}

std::string variable::DefaultValue_ToString() const noexcept {
	std::string str;

	switch (type_ & nptype::TYPE_MASK) {
	case nptype::NPT_UNDEFINE:
		str = "***UNDEFINED TYPE***";
		break;
	case nptype::NPT_BOOL:
		if (def_value_._b == true)
			str = "1";
		else
			str = "0";
		break;
	case nptype::NPT_U8:
		str = std::to_string(def_value_._u8);
		break;
	case nptype::NPT_I8:
		str = std::to_string(def_value_._i8);
		break;
	case nptype::NPT_U16:
		str = std::to_string(def_value_._u16);
		break;
	case nptype::NPT_I16:
		str = std::to_string(def_value_._i16);
		break;
	case nptype::NPT_U32:
		str = std::to_string(def_value_._u32);
		break;
	case nptype::NPT_I32:
		str = std::to_string(def_value_._i32);
		break;
	case nptype::NPT_F32:
		str = std::to_string(def_value_._f32);
		break;
	default:
		ASSERT(FALSE);
		break;
	}
	return str;
}

void variable::DefaultValue_FromString(const std::string& str) {
	switch (type_ & nptype::TYPE_MASK) {
	case nptype::NPT_UNDEFINE:
		break;
	case nptype::NPT_BOOL:
		ParseTextValue(str.c_str(), str.length(), def_value_._b);
		break;
	case nptype::NPT_U8:
		ParseTextValue(str.c_str(), str.length(), def_value_._u8);
		break;
	case nptype::NPT_I8:
		ParseTextValue(str.c_str(), str.length(), def_value_._i8);
		break;
	case nptype::NPT_U16:
		ParseTextValue(str.c_str(), str.length(), def_value_._u16);
		break;
	case nptype::NPT_I16:
		ParseTextValue(str.c_str(), str.length(), def_value_._i16);
		break;
	case nptype::NPT_U32:
		ParseTextValue(str.c_str(), str.length(), def_value_._u32);
		break;
	case nptype::NPT_I32:
		ParseTextValue(str.c_str(), str.length(), def_value_._i32);
		break;
	case nptype::NPT_F32:
		ParseTextValue(str.c_str(), str.length(), def_value_._f32);
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}

int variable::OwnerRelease(odb::NodeListMementoManager* mm) noexcept {
	ASSERT(!IsIO());

	if (id() == odb::NEW_NODE_ID) {
		ASSERT(ref_cnt_ == 0);
		return 0;
	} 

	if (ref_cnt_ == 0) {
		ASSERT(status_ < Status::good); // loaded variable cannot have ref_cnt = 0
		auto self = self_node.fetch();
		self.remove();
		return 0;
	}

	ASSERT(status_ < Status::to_remove);

	set_modified();
	--ref_cnt_;
	
	ASSERT(id() != odb::NEW_NODE_ID);
	ASSERT(status_ >= Status::good);

	status_ = Status::to_remove;
	
	auto device = dev_.fetch();
	
	ASSERT(device.loaded());
	
	device->allocated_vars_.fetch_all_nodes();
	device->vars_owner_released_.fetch_all_nodes();
	
	auto self = self_node.fetch();
	
	if (mm) {
		mm->AddMemento(device->allocated_vars_);
		mm->AddMemento(device->vars_owner_released_);
		device->allocated_vars_.erase(self);
		device->vars_owner_released_.add_node(self);
	} else {
		device->allocated_vars_.erase(self);
		device->vars_owner_released_.add_node(self);
		device->allocated_vars_.store();
		device->vars_owner_released_.store();
		self.store();
	}

	return ref_cnt_;
}

} // namespace npsys