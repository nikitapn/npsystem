// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#ifndef __SERVER_VARIANT_H__
#define __SERVER_VARIANT_H__

#include <npc/server.hpp>
#include <npsys/memtypes.h>
#include <npsys/variable.h>

class online_value {
	using variable = npsys::variable;
	using type_t = npsys::nptype::Type;
private:
	int type_;
	nps::server_value val_;

	void validate_type(type_t type, bool quality) const
	{
	//	if (val_.q == cb::quality::QT_DEVICE_NOT_RESPONSING || val_.q == cb::quality::QT_UNKNOWN)
	//		throw std::runtime_error("bad value state");
		if (variable::IsQuality(type_) != quality || variable::GetClearType(type_) != type)
			throw std::bad_cast();
	}
public:
	online_value() noexcept {
		type_ = type_t::NPT_UNDEFINE;
		val_.s = nps::var_status::VST_UNKNOWN;
	}
	explicit online_value(int type) noexcept {
		type_ = type;
		val_.s = nps::var_status::VST_UNKNOWN;
	}
	int get_type() const noexcept {
		return type_;
	}
	void set_type(int type) noexcept {
		type_ = type;
		val_.s = nps::var_status::VST_UNKNOWN;
	}
	void operator = (const nps::server_value& value) noexcept {
		val_ = value;
	}
	void operator = (const void* data) noexcept {
		std::memcpy(&val_, data, sizeof(nps::server_value));
	}
	bool is_discrete() const {
		return variable::IsBit(type_);
	}
//	bool is_qbit() const {
//		return variable::IsQuality(type_);
//	}
	bool is_quality() const {
		return variable::IsQuality(type_);
	}
	// bit
	const npsystem::types::BIT& to_bit() const {
		validate_type(type_t::NPT_BOOL, false);
		return reinterpret_cast<const npsystem::types::BIT&>(val_.data[0]);
	}
	const npsystem::types::QBIT& to_Qbit() const {
		validate_type(type_t::NPT_BOOL, true);
		return reinterpret_cast<const npsystem::types::QBIT&>(val_.data[0]);
	}
	//8
	const npsystem::types::U8& to_u8() const {
		validate_type(type_t::NPT_U8, false);
		return reinterpret_cast<const npsystem::types::U8&>(val_.data[0]);
	}
	const npsystem::types::QU8& to_Qu8() const {
		validate_type(type_t::NPT_U8, true);
		return reinterpret_cast<const npsystem::types::QU8&>(val_.data[0]);
	}
	const npsystem::types::I8& to_i8() const {
		validate_type(type_t::NPT_I8, false);
		return reinterpret_cast<const npsystem::types::I8&>(val_.data[0]);
	}
	const npsystem::types::QI8& to_Qi8() const {
		validate_type(type_t::NPT_I8, true);
		return reinterpret_cast<const npsystem::types::QI8&>(val_.data[0]);
	}
	// 16
	const npsystem::types::U16& to_u16() const {
		validate_type(type_t::NPT_U16, false);
		return reinterpret_cast<const npsystem::types::U16&>(val_.data[0]);
	}
	const npsystem::types::QU16& to_Qu16() const {
		validate_type(type_t::NPT_U16, true);
		return reinterpret_cast<const npsystem::types::QU16&>(val_.data[0]);
	}
	const npsystem::types::I16& to_i16() const {
		validate_type(type_t::NPT_I16, false);
		return reinterpret_cast<const npsystem::types::I16&>(val_.data[0]);
	}
	const npsystem::types::QI16& to_Qi16() const {
		validate_type(type_t::NPT_I16, true);
		return reinterpret_cast<const npsystem::types::QI16&>(val_.data[0]);
	}
	// 32
	const npsystem::types::U32& to_u32() const {
		validate_type(type_t::NPT_U32, false);
		return reinterpret_cast<const npsystem::types::U32&>(val_.data[0]);
	}
	const npsystem::types::QU32& to_Qu32() const {
		validate_type(type_t::NPT_U32, true);
		return reinterpret_cast<const npsystem::types::QU32&>(val_.data[0]);
	}
	const npsystem::types::I32& to_i32() const {
		validate_type(type_t::NPT_I32, false);
		return reinterpret_cast<const npsystem::types::I32&>(val_.data[0]);
	}
	const npsystem::types::QI32& to_Qi32() const {
		validate_type(type_t::NPT_I32, true);
		return reinterpret_cast<const npsystem::types::QI32&>(val_.data[0]);
	}
	// float
	const npsystem::types::F32& to_flt() const {
		validate_type(type_t::NPT_F32, false);
		return reinterpret_cast<const npsystem::types::F32&>(val_.data[0]);
	}
	const npsystem::types::QF32& to_Qflt() const {
		validate_type(type_t::NPT_F32, true);
		return reinterpret_cast<const npsystem::types::QF32&>(val_.data[0]);
	}
	void zero_memory() noexcept {
		std::fill(val_.data.begin(), val_.data.end(), 0x00);
	}
	std::string to_influx_db() const noexcept;
	
	std::string to_string(int precision) const noexcept {
		return to_string_impl(precision, false);
	}
	std::string to_string_show_quality(int precision) const noexcept {
		return to_string_impl(precision, true);
	}
	bool is_quality_not_good() const noexcept;
private:
	std::string to_string_impl(int precision, bool show_quality) const noexcept;
};

#endif // !__SERVER_VARIANT_H__