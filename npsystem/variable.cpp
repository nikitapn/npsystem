// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include <npsys/variable.h>
#include "network_ext.h"
#include "assignedalgorithm.h"
#include <npdb/memento_manager.h>

namespace npsys {

void variable::m_DefaultValue_Clear(Type tp) noexcept {
	switch (tp) {
	case VT_DISCRETE:
		def_value_.d = static_cast<discrete>(0);
		break;
	case VT_BYTE:
		def_value_.u8 = static_cast<byte>(0);
		break;
	case VT_SIGNED_BYTE:
		def_value_.i8 = static_cast<signed_byte>(0);
		break;
	case VT_WORD:
		def_value_.u16 = static_cast<word>(0);
		break;
	case VT_SIGNED_WORD:
		def_value_.i16 = static_cast<signed_word>(0);
		break;
	case VT_DWORD:
		def_value_.u32 = static_cast<dword>(0);
		break;
	case VT_SIGNED_DWORD:
		def_value_.i32 = static_cast<signed_dword>(0);
		break;
	case VT_FLOAT:
		def_value_.flt = static_cast<floating_point>(0.0f);
		break;
	default:
		memset(def_value_.data, 0x00, sizeof(def_value_.data));
		break;
	}
}

std::string variable::DefaultValue_ToString() const noexcept {
	std::string str;

	switch (type_ & TYPE_MASK) {
	case VT_UNDEFINE:
		str = "***UNDEFINED TYPE***";
		break;
	case VT_DISCRETE:
		if (def_value_.d == true)
			str = "1";
		else
			str = "0";
		break;
	case VT_BYTE:
		str = std::to_string(def_value_.u8);
		break;
	case VT_SIGNED_BYTE:
		str = std::to_string(def_value_.i8);
		break;
	case VT_WORD:
		str = std::to_string(def_value_.u16);
		break;
	case VT_SIGNED_WORD:
		str = std::to_string(def_value_.i16);
		break;
	case VT_DWORD:
		str = std::to_string(def_value_.u32);
		break;
	case VT_SIGNED_DWORD:
		str = std::to_string(def_value_.i32);
		break;
	case VT_FLOAT:
		str = std::to_string(def_value_.flt);
		break;
	default:
		ASSERT(FALSE);
		break;
	}
	return str;
}

void variable::DefaultValue_FromString(const std::string& str) {
	switch (type_ & TYPE_MASK) {
	case VT_UNDEFINE:
		break;
	case VT_DISCRETE:
		ParseTextValue(str.c_str(), str.length(), def_value_.d);
		break;
	case VT_BYTE:
		ParseTextValue(str.c_str(), str.length(), def_value_.u8);
		break;
	case VT_SIGNED_BYTE:
		ParseTextValue(str.c_str(), str.length(), def_value_.i8);
		break;
	case VT_WORD:
		ParseTextValue(str.c_str(), str.length(), def_value_.u16);
		break;
	case VT_SIGNED_WORD:
		ParseTextValue(str.c_str(), str.length(), def_value_.i16);
		break;
	case VT_DWORD:
		ParseTextValue(str.c_str(), str.length(), def_value_.u32);
		break;
	case VT_SIGNED_DWORD:
		ParseTextValue(str.c_str(), str.length(), def_value_.i32);
		break;
	case VT_FLOAT:
		ParseTextValue(str.c_str(), str.length(), def_value_.flt);
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