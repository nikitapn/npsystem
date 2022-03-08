// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#ifndef __VARIABLE_H__
#define __VARIABLE_H__

#include "assert.h"
#include "header.h"


#include <nplib/utils/value_parser.h>


#include <ostream>
#include <npdb/memento_manager.h>
#include <npsys/other/uploadable.h>

namespace npsys {
class CAssignedAlgorithm;
class variable
	: public odb::common_interface<odb::no_interface>
	, public odb::self_node_member<variable_n>
	, public odb::modified_flag
	, public IUploadable {
	friend std::ostream& operator<<(std::ostream&, const variable&);
	friend CAssignedAlgorithm;
public:
	static constexpr int VQUALITY = 0x00002000;
	static constexpr int IO_SPACE = 0x00004000;
	static constexpr int MUTABLE = 0x00008000;
	static constexpr int SIGNED = 0x00001000;
	static constexpr int INTERNAL = 0x00010000;
	static constexpr int SIZE_8 = 0x00000001;
	static constexpr int SIZE_16 = 0x00000002;
	static constexpr int SIZE_32 = 0x00000004;
	static constexpr int FLOAT_VALUE = 0x00000010;
	static constexpr int BIT_VALUE = 0x00000020;
	static constexpr int INT_VALUE = 0x00000040;
	static constexpr int BIT_MASK = 0x00000F00;
	static constexpr int SIZE_MASK = 0x0000000F;
	static constexpr int TYPE_MASK = BIT_VALUE | INT_VALUE | FLOAT_VALUE
		| SIGNED | SIZE_8 | SIZE_16 | SIZE_32;
public:
	using discrete = bool;
	using byte = unsigned char;
	using signed_byte = char;
	using word = unsigned short;
	using signed_word = short;
	using dword = unsigned int;
	using signed_dword = int;
	using floating_point = float;
	union Value {
		discrete d;
		byte u8;
		signed_byte i8;
		word u16;
		signed_word i16;
		dword u32;
		signed_dword i32;
		floating_point flt;
		uint8_t data[4];
	};
	enum Type {
		VT_UNDEFINE = (0x00000000),
		VT_DISCRETE = (SIZE_8 | BIT_VALUE),
		VT_BYTE = (SIZE_8 | INT_VALUE),
		VT_SIGNED_BYTE = (SIGNED | SIZE_8 | INT_VALUE),
		VT_WORD = (SIZE_16 | INT_VALUE),
		VT_SIGNED_WORD = (SIGNED | SIZE_16 | INT_VALUE),
		VT_DWORD = (SIZE_32 | INT_VALUE),
		VT_SIGNED_DWORD = (SIGNED | SIZE_32 | INT_VALUE),
		VT_FLOAT = (SIGNED | SIZE_32 | FLOAT_VALUE)
	};
	enum class Status {
		not_loaded,
		allocated,
		allocated_from_another_algorithm,
		good,
		good_allocated_from_another_algorithm,
		to_remove,
		removed_refcnt_not_null
	};

private:
	int32_t addr_;
	int32_t type_;
	Status status_;
	Value def_value_;
	bool def_value_modified_;

	mutable odb::weak_node<device_n> dev_;
	mutable odb::weak_node<control_unit_n> alg_;

	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {
		ar& dev_;
		ar& alg_;
		ar& addr_;
		ar& type_;
		ar& ref_cnt_;
		ar& status_;
		ar& def_value_.data;
		ar& def_value_modified_;
	}
protected:
	int32_t ref_cnt_;
public:
	variable() = default;
	explicit variable(int type) noexcept;
	explicit variable(const variable& other) noexcept;
	void reset() noexcept;
	std::string to_ctype() const noexcept;
	std::string to_vardecl() const noexcept;
	std::string to_vardeclq() const noexcept;

	constexpr static bool TypesEqual(int type_1, int type_2) noexcept;
	constexpr static bool IsBit(int type) noexcept;
	constexpr static bool IsIO(int type) noexcept;
	constexpr static bool IsSigned(int type) noexcept;
	constexpr static bool IsQuality(int type) noexcept;
	constexpr static bool IsMutableType(int type) noexcept;
	constexpr static bool IsInternalType(int type) noexcept;
	constexpr static int GetBit(int type) noexcept;
	constexpr static int GetSize(int type) noexcept;
	constexpr static int GetSizeWithQuality(int type) noexcept;
	constexpr static Type GetClearType(int type) noexcept;

	bool IsBit() const noexcept;
	bool IsIO() const noexcept;
	bool IsSigned() const noexcept;
	bool IsQuality() const noexcept;
	bool IsMutableType() const noexcept;
	bool IsInternalType() const noexcept;
	bool IsGood() const noexcept;
	int GetBit() const noexcept;
	int GetSize() const noexcept;
	int GetSizeWithQuality() const noexcept;
	int GetAddr() const noexcept;
	int GetDevAddr() const noexcept;
	int GetType() const noexcept;
	Status GetStatus() const noexcept;
	Type GetClearType() const noexcept;
	int GetQBit() const noexcept;
	int GetQAddr() const noexcept;
	void SetType(int type) noexcept;
	void SetBit(int bit) noexcept;
	void SetAddr(int addr) noexcept;
	const odb::weak_node<control_unit_n>& GetAlg() const noexcept;
	void SetAlg(odb::weak_node<control_unit_n> alg) noexcept;
	const odb::weak_node<device_n>& GetDev() const noexcept;
	void SetDev(odb::weak_node<device_n> dev) noexcept;
	void SetStatus(Status status) noexcept;
	int AddRef() noexcept;
	int OwnerRelease(odb::NodeListMementoManager* mm) noexcept;
	int RefCount() const noexcept;
	void DefaultValue_SetValue(discrete value) noexcept;
	void DefaultValue_SetValue(byte value) noexcept;
	void DefaultValue_SetValue(signed_byte value) noexcept;
	void DefaultValue_SetValue(word value) noexcept;
	void DefaultValue_SetValue(signed_word value) noexcept;
	void DefaultValue_SetValue(dword value) noexcept;
	void DefaultValue_SetValue(signed_dword value) noexcept;
	void DefaultValue_SetValue(floating_point value) noexcept;
	Value DefaultValue_GetValue() const noexcept;
	std::string PrintStatus() const noexcept;
	virtual std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept final {
		return odb::MakeMemento(*this, addr_, type_, ref_cnt_, status_);
	}

	std::string DefaultValue_ToString() const noexcept;
	void DefaultValue_FromString(const std::string& str) ;
	bool DefaultValue_Modified() const;
	void DefaultValue_SetModified(bool modified);
private:
	void m_DefaultValue_Clear(Type tp) noexcept;
};



} // namespace npsys

#include "variable.inl"

using var_list = std::vector<npsys::variable_n>;
using var_node_ptr_list = std::vector<npsys::variable_n*>;
using var_raw_ptr_list = std::vector<npsys::variable*>;

#endif // !__VARIABLE_H__