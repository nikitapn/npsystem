#pragma once

#include "../avr_firmware/src/twi.h"

std::string vt_to_string(cb::variant_type vt);

class wrong_type : public std::exception
{
public:
	explicit wrong_type(cb::variant_type vt) 
		: vt_(vt) { }
	virtual char const* what() const
	{
		return ("The value type is " + vt_to_string(vt_)).c_str();
	}
private:
	cb::variant_type vt_;
};

class server_variant : public cb::variant
{
public:
	explicit server_variant() {
		this->vt = cb::variant_type::DT_UNKNOWN;
	}
	explicit server_variant(cb::variant_type vt) {
		this->vt = vt;
	}
	bool is_bit() const
	{
		return vt >= cb::variant_type::DT_BIT_0 && vt <= cb::variant_type::DT_BIT_7;
	}
	bool is_qbit() const
	{
		return vt >= cb::variant_type::DT_BIT_0_QUALITY && vt <= cb::variant_type::DT_BIT_6_QUALITY;
	}
	// bit
	bit to_bit() const
	{
		if (!is_bit())
			throw std::bad_cast(); 
		int bit_n = (int)(vt - cb::variant_type::DT_BIT_0);
		bit bt; 
		bt.value = data[0] & (1 << bit_n) ? true : false;
		return bt; 
	}
	Q_bit to_Qbit() const
	{
		if (!is_qbit())
			throw std::bad_cast();
		int bit_n = 2 * (int)(vt - cb::variant_type::DT_BIT_0_QUALITY);
		Q_bit bt;
		bt.value = data[0] & (1 << bit_n) ? true : false;
		bt.quality = data[0] & ( 2 << bit_n) ? true : false;
		return bt;
	}
	//8
	const u8& to_u8() const {
		if (vt != cb::variant_type::DT_BYTE)
			throw std::bad_cast();
		return reinterpret_cast<const u8&>(data[0]);
	}
	const Q_u8& to_Qu8() const {
		if (vt != cb::variant_type::DT_BYTE_QUALITY)
			throw std::bad_cast();
		return reinterpret_cast<const Q_u8&>(data[0]);
	}
	const i8& to_i8() const {
		if (vt != cb::variant_type::DT_SIGNED_BYTE)
			throw std::bad_cast();
		return reinterpret_cast<const i8&>(data[0]);
	}
	const Q_i8& to_Qi8() const {
		if (vt != cb::variant_type::DT_SIGNED_BYTE_QUALITY)
			throw std::bad_cast();
		return reinterpret_cast<const Q_i8&>(data[0]);
	}
	// 16
	const u16& to_u16() const {
		if (vt != cb::variant_type::DT_WORD)
			throw std::bad_cast();
		return reinterpret_cast<const u16&>(data[0]);
	}
	const Q_u16& to_Qu16() const {
		if (vt != cb::variant_type::DT_WORD_QUALITY)
			throw std::bad_cast();
		return reinterpret_cast<const Q_u16&>(data[0]);
	}
	const i16& to_i16() const {
		if (vt != cb::variant_type::DT_SIGNED_WORD)
			throw std::bad_cast();
		return reinterpret_cast<const i16&>(data[0]);
	}
	const Q_i16& to_Qi16() const {
		if (vt != cb::variant_type::DT_SIGNED_WORD_QUALITY)
			throw std::bad_cast();
		return reinterpret_cast<const Q_i16&>(data[0]);
	}
	// 32
	const u32& to_u32() const {
		if (vt != cb::variant_type::DT_DWORD)
			throw std::bad_cast();
		return reinterpret_cast<const u32&>(data[0]);
	}
	const Q_u32& to_Qu32() const {
		if (vt != cb::variant_type::DT_DWORD_QUALITY)
			throw std::bad_cast();
		return reinterpret_cast<const Q_u32&>(data[0]);
	}
	const i32& to_i32() const {
		if (vt != cb::variant_type::DT_SIGNED_DWORD)
			throw std::bad_cast();
		return reinterpret_cast<const i32&>(data[0]);
	}
	const Q_i32& to_Qi32() const {
		if (vt != cb::variant_type::DT_SIGNED_DWORD_QUALITY)
			throw std::bad_cast();
		return reinterpret_cast<const Q_i32&>(data[0]);
	}
	// float
	const flt& to_flt() const {
		if (vt != cb::variant_type::DT_FLOAT)
			throw std::bad_cast();
		return reinterpret_cast<const flt&>(data[0]);
	}
	const Q_flt& to_Qflt() const {
		if (vt != cb::variant_type::DT_FLOAT_QUALITY)
			throw std::bad_cast();
		return reinterpret_cast<const Q_flt&>(data[0]);
	}
	// true - good quality
	bool quality() const {
		switch (vt)
		{
		case cb::variant_type::DT_UNKNOWN:
			throw wrong_type(cb::variant_type::DT_UNKNOWN);
		case cb::variant_type::DT_BIT_0:
			throw wrong_type(cb::variant_type::DT_BIT_0);
		case cb::variant_type::DT_BIT_1:
			throw wrong_type(cb::variant_type::DT_BIT_1);
		case cb::variant_type::DT_BIT_2:
			throw wrong_type(cb::variant_type::DT_BIT_2);
		case cb::variant_type::DT_BIT_3:
			throw wrong_type(cb::variant_type::DT_BIT_3);
		case cb::variant_type::DT_BIT_4:
			throw wrong_type(cb::variant_type::DT_BIT_4);
		case cb::variant_type::DT_BIT_5:
			throw wrong_type(cb::variant_type::DT_BIT_5);
		case cb::variant_type::DT_BIT_6:
			throw wrong_type(cb::variant_type::DT_BIT_6);
		case cb::variant_type::DT_BIT_7:
			throw wrong_type(cb::variant_type::DT_BIT_7);
		case cb::variant_type::DT_BYTE:
			throw wrong_type(cb::variant_type::DT_BYTE);
		case cb::variant_type::DT_SIGNED_BYTE:
			throw wrong_type(cb::variant_type::DT_SIGNED_BYTE);
		case cb::variant_type::DT_WORD:
			throw wrong_type(cb::variant_type::DT_WORD);
		case cb::variant_type::DT_SIGNED_WORD:
			throw wrong_type(cb::variant_type::DT_SIGNED_WORD);
		case cb::variant_type::DT_DWORD:
			throw wrong_type(cb::variant_type::DT_DWORD);
		case cb::variant_type::DT_SIGNED_DWORD:
			throw wrong_type(cb::variant_type::DT_SIGNED_DWORD);
		case cb::variant_type::DT_FLOAT:
			throw wrong_type(cb::variant_type::DT_FLOAT);
		case cb::variant_type::DT_BIT_0_QUALITY:
		case cb::variant_type::DT_BIT_2_QUALITY:
		case cb::variant_type::DT_BIT_4_QUALITY:
		case cb::variant_type::DT_BIT_6_QUALITY:
		{
			const Q_bit& bt = to_Qbit();
			return static_cast<bool>(bt.quality);
		}
		case cb::variant_type::DT_BYTE_QUALITY:
		{
			const Q_u8& val = to_Qu8();
			return static_cast<bool>(val.quality);
		}
		case cb::variant_type::DT_SIGNED_BYTE_QUALITY:
		{
			const Q_i8& val = to_Qi8();
			return static_cast<bool>(val.quality);
		}
		case cb::variant_type::DT_WORD_QUALITY:
		{
			const Q_u16& val = to_Qu16();
			return static_cast<bool>(val.quality);
		}
		case cb::variant_type::DT_SIGNED_WORD_QUALITY:
		{
			const Q_i16& val = to_Qi16();
			return static_cast<bool>(val.quality);
		}
		case cb::variant_type::DT_DWORD_QUALITY:
		{
			const Q_u32& val = to_Qu32();
			return static_cast<bool>(val.quality);
		}
		case cb::variant_type::DT_SIGNED_DWORD_QUALITY:
		{
			const Q_i32& val = to_Qi32();
			return static_cast<bool>(val.quality);

		}
		case cb::variant_type::DT_FLOAT_QUALITY:
		{
			const Q_flt& val = to_Qflt();
			return static_cast<bool>(val.quality);
		}
		default:
			throw wrong_type(cb::variant_type::DT_UNKNOWN);
			return false;
		};
	}
	/*
	bool equal(const server_variant& other) const {
		return *(uint64_t*)&data[0] == *(uint64_t*)&other.data[0];
	}
	bool operator == (const server_variant& other) const {
		return equal(other);
	}
	bool operator != (const server_variant& other) const {
		return !equal(other);
	}
	*/
};