// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#ifndef __SERVER_VARIANT_H__
#define __SERVER_VARIANT_H__

#include <npc/server.hpp>
#include <npsys/memtypes.h>
#include <npsys/variable.h>

class online_value
{
	using variable = npsys::variable;
	using type_t = variable::Type;
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
		type_ = type_t::VT_UNDEFINE;
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
	const bit& to_bit() const {
		validate_type(type_t::VT_DISCRETE, false);
		return reinterpret_cast<const bit&>(val_.data[0]);
	}
	const Q_bit& to_Qbit() const {
		validate_type(type_t::VT_DISCRETE, true);
		return reinterpret_cast<const Q_bit&>(val_.data[0]);
	}
	//8
	const u8& to_u8() const {
		validate_type(type_t::VT_BYTE, false);
		return reinterpret_cast<const u8&>(val_.data[0]);
	}
	const Q_u8& to_Qu8() const {
		validate_type(type_t::VT_BYTE, true);
		return reinterpret_cast<const Q_u8&>(val_.data[0]);
	}
	const i8& to_i8() const {
		validate_type(type_t::VT_SIGNED_BYTE, false);
		return reinterpret_cast<const i8&>(val_.data[0]);
	}
	const Q_i8& to_Qi8() const {
		validate_type(type_t::VT_SIGNED_BYTE, true);
		return reinterpret_cast<const Q_i8&>(val_.data[0]);
	}
	// 16
	const u16& to_u16() const {
		validate_type(type_t::VT_WORD, false);
		return reinterpret_cast<const u16&>(val_.data[0]);
	}
	const Q_u16& to_Qu16() const {
		validate_type(type_t::VT_WORD, true);
		return reinterpret_cast<const Q_u16&>(val_.data[0]);
	}
	const i16& to_i16() const {
		validate_type(type_t::VT_SIGNED_WORD, false);
		return reinterpret_cast<const i16&>(val_.data[0]);
	}
	const Q_i16& to_Qi16() const {
		validate_type(type_t::VT_SIGNED_WORD, true);
		return reinterpret_cast<const Q_i16&>(val_.data[0]);
	}
	// 32
	const u32& to_u32() const {
		validate_type(type_t::VT_DWORD, false);
		return reinterpret_cast<const u32&>(val_.data[0]);
	}
	const Q_u32& to_Qu32() const {
		validate_type(type_t::VT_DWORD, true);
		return reinterpret_cast<const Q_u32&>(val_.data[0]);
	}
	const i32& to_i32() const {
		validate_type(type_t::VT_SIGNED_DWORD, false);
		return reinterpret_cast<const i32&>(val_.data[0]);
	}
	const Q_i32& to_Qi32() const {
		validate_type(type_t::VT_SIGNED_DWORD, true);
		return reinterpret_cast<const Q_i32&>(val_.data[0]);
	}
	// float
	const flt& to_flt() const {
		validate_type(type_t::VT_FLOAT, false);
		return reinterpret_cast<const flt&>(val_.data[0]);
	}
	const Q_flt& to_Qflt() const {
		validate_type(type_t::VT_FLOAT, true);
		return reinterpret_cast<const Q_flt&>(val_.data[0]);
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