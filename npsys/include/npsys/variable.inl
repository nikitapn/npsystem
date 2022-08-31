// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <iostream>
#include <iomanip>
#include <assert.h>

namespace npsys {
inline variable::variable(int type) noexcept 
	: addr_(0)
	, type_(type)
	, status_(Status::not_loaded)
	, def_value_modified_(true)
	, ref_cnt_(0)
{
	m_DefaultValue_Clear(variable::GetClearType(type_));
}

inline variable::variable(const variable& other) noexcept 
	: addr_(0)
	, status_(Status::not_loaded)
	, def_value_modified_(true)
	, ref_cnt_(0) 
{
	type_ = other.GetType();
	def_value_ = other.def_value_;
}

inline void variable::reset() noexcept {
	dev_ = {};
	alg_ = {};
	status_ = Status::not_loaded;
	ref_cnt_ = 0;
	def_value_modified_ = true; // always write eeprom first time even it is not a parameter variable
}
// static
inline constexpr bool variable::IsSigned(int type) noexcept {
	return (type & SIGNED) ? true : false;
}
inline constexpr bool variable::TypesEqual(int type_1, int type_2) noexcept {
	return (type_1 & ~BIT_MASK) == (type_2 & ~BIT_MASK);
}
inline constexpr bool variable::IsQuality(int type) noexcept {
	return (type & VQUALITY) ? true : false;
}
inline constexpr bool variable::IsMutableType(int type) noexcept {
	return (type & MUTABLE) ? true : false;
}
inline constexpr bool variable::IsInternalType(int type) noexcept {
	return (type & variable::INTERNAL) ? true : false;
}
inline constexpr bool variable::IsIO(int type) noexcept {
	return (type & IO_SPACE) ? true : false;
}
inline constexpr bool variable::IsBit(int type) noexcept {
	return (type & BIT_VALUE) ? true : false;
}
inline constexpr int variable::GetBit(int type) noexcept {
	assert(variable::IsBit(type));
	return (type & 0xF00) >> 8;
}
inline constexpr int variable::GetSize(int type) noexcept {
	return (type & SIZE_MASK);
}
inline constexpr int variable::GetSizeWithQuality(int type) noexcept {
	return GetSize(type) + (IsBit(type) ? 0 : (IsQuality(type) ? 1 : 0));
}
inline constexpr variable::Type variable::GetClearType(int type) noexcept {
	return static_cast<variable::Type>(type & TYPE_MASK);
}
// static end
inline bool variable::IsBit() const noexcept {
	return variable::IsBit(type_);
}
inline bool variable::IsIO() const noexcept {
	return variable::IsIO(type_);
}
inline bool variable::IsQuality() const noexcept {
	return variable::IsQuality(type_);
}
inline bool variable::IsMutableType() const noexcept {
	return variable::IsMutableType(type_);
}
inline bool variable::IsInternalType() const noexcept {
	return variable::IsInternalType(type_);
}
inline bool variable::IsSigned() const noexcept {
	return variable::IsSigned(type_);
}
inline bool variable::IsGood() const noexcept {
	return ((status_ >= Status::good) && (status_ <= Status::good_allocated_from_another_algorithm));
}
inline int variable::GetType() const noexcept {
	return type_;
}
inline variable::Status variable::GetStatus() const noexcept {
	return status_;
}
inline variable::Type variable::GetClearType() const noexcept {
	return variable::GetClearType(type_);
}
inline int variable::GetBit() const noexcept {
	return variable::GetBit(type_);
}
inline int variable::GetQBit() const noexcept {
	assert(IsBit(type_));
	assert(IsQuality());
	assert(!IsIO(type_));
	return GetBit(type_) + 1;
}
inline int variable::GetSize() const noexcept {
	return variable::GetSize(type_);
}
inline int variable::GetSizeWithQuality() const noexcept {
	return variable::GetSizeWithQuality(type_);
}
inline int variable::GetAddr() const noexcept {
	return addr_;
}
inline int variable::GetQAddr() const noexcept {
	assert(IsQuality());
	return (IsBit() ? addr_ : (addr_ + GetSize()));
}
inline std::string variable::to_vardecl() const noexcept {
	return "_" + std::to_string(self_node.id());
}
inline std::string variable::to_vardeclq() const noexcept {
	return to_vardecl() + "_q";
}
inline void variable::SetAddr(int addr) noexcept {
	this->addr_ = addr;
	set_modified();
}
inline const odb::weak_node<device_n>& variable::GetDev() const noexcept {
	return dev_;
}
inline void variable::SetDev(odb::weak_node<device_n> dev) noexcept {
	dev_ = dev;
	set_modified();
}
inline const odb::weak_node<control_unit_n>& variable::GetAlg() const noexcept {
	return alg_;
}
inline void variable::SetAlg(odb::weak_node<control_unit_n> unit) noexcept {
	alg_ = unit;
	set_modified();
}
inline void variable::SetBit(int bit) noexcept {
	assert(IsBit());
	type_ &= ~BIT_MASK;
	type_ |= bit << 8;
	set_modified();
}
inline void variable::SetType(int type) noexcept {
	if (type_ != type) {
		def_value_modified_ = true;
		m_DefaultValue_Clear(variable::GetClearType(type));
	}
	type_ = type;
	set_modified();
}
inline void variable::SetStatus(Status status) noexcept {
	status_ = status;
	set_modified();
}
inline int variable::AddRef() noexcept {
	set_modified();
	return ++ref_cnt_;
}
inline int variable::RefCount() const noexcept {
	return ref_cnt_;
}
inline variable::Value variable::DefaultValue_GetValue() const noexcept {
	return def_value_;
}
inline void variable::DefaultValue_SetModified(bool modified) {
	if (def_value_modified_ == modified) return;
	def_value_modified_ = modified;
	set_modified();
}
inline bool variable::DefaultValue_Modified() const {
	return def_value_modified_;
}
inline void variable::DefaultValue_SetValue(variable::discrete value) noexcept {
	def_value_.d = value;
	DefaultValue_SetModified(true);
}
inline void variable::DefaultValue_SetValue(variable::byte value) noexcept {
	def_value_.u8 = value;
	DefaultValue_SetModified(true);
}
inline void variable::DefaultValue_SetValue(variable::signed_byte value) noexcept {
	def_value_.i8 = value;
	DefaultValue_SetModified(true);
}
inline void variable::DefaultValue_SetValue(variable::word value) noexcept {
	def_value_.u16 = value;
	DefaultValue_SetModified(true);
}
inline void variable::DefaultValue_SetValue(variable::signed_word value) noexcept {
	def_value_.i16 = value;
	DefaultValue_SetModified(true);
}
inline void variable::DefaultValue_SetValue(variable::dword value) noexcept {
	def_value_.u32 = value;
	DefaultValue_SetModified(true);
}
inline void variable::DefaultValue_SetValue(variable::signed_dword value) noexcept {
	def_value_.i32 = value;
	DefaultValue_SetModified(true);
}
inline void variable::DefaultValue_SetValue(variable::floating_point value) noexcept {
	def_value_.flt = value;
	DefaultValue_SetModified(true);
}

inline std::ostream& operator<<(std::ostream& os, const variable& v) {
	os << "var - id: " << v.id() << ", " << v.to_ctype() <<
		", a: 0x" << std::hex << std::setfill('0') << std::setw(4) << v.GetAddr() << std::dec;
	if (v.IsBit()) {
		os << ", bn: " << v.GetBit();
		if (v.IsQuality()) os << " qbn: " << v.GetQBit();
	}
	os << ", s: " << v.PrintStatus() <<
		", dev_addr: " << v.GetDevAddr() << ", alg_id: " << v.GetAlg().id() << ", ref_cnt: " << v.ref_cnt_;
	return os;
}

inline constexpr std::string_view variable::to_ctype(int type) noexcept {
	using namespace std::string_view_literals;
	switch (GetClearType(type)) {
	case VT_DISCRETE:
		return "discrete_t"sv;
	case VT_BYTE:
		return "uint8_t"sv;
	case VT_SIGNED_BYTE:
		return "int8_t"sv;
	case VT_WORD:
		return "uint16_t"sv;
	case VT_SIGNED_WORD:
		return "int16_t"sv;
	case VT_DWORD:
		return "uint32_t"sv;
	case VT_SIGNED_DWORD:
		return "int32_t"sv;
	case VT_FLOAT:
		return "float"sv;
	default:
		return "unknown_type"sv;
	}
}

inline std::string variable::to_ctype() const noexcept {
	return std::string(variable::to_ctype(type_));
}

inline std::string variable::PrintStatus() const noexcept {
	switch (status_)
	{
	case npsys::variable::Status::not_loaded:
		return std::string("nl");
	case npsys::variable::Status::allocated:
		return std::string("al");
	case npsys::variable::Status::allocated_from_another_algorithm:
		return std::string("alfa");
	case npsys::variable::Status::good:
		return std::string("g");
	case npsys::variable::Status::good_allocated_from_another_algorithm:
		return std::string("gfa");
	case npsys::variable::Status::to_remove:
		return std::string("tr");
	case npsys::variable::Status::removed_refcnt_not_null:
		return std::string("rcnll");
	default:
		return std::string("!!!UNKNOWN!!!");
	}
}


}; // namespace npsys
