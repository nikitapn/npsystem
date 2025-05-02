// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#ifndef __VARIABLE_H__
#define __VARIABLE_H__

#include "assert.h"
#include "header.h"
#include "fundamental.h"

#include <nplib/utils/value_parser.h>
#include <nplib/utils/types.h>


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
	union DefaultValue {
		bool _b;
		u8 _u8; 
		i8 _i8;
		u16 _u16; 
		i16 _i16;
		u32 _u32; 
		i32 _i32;
		f32 _f32;
		uint8_t data[4];
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
	DefaultValue def_value_;
	bool def_value_modified_;

	mutable odb::weak_node<device_n> dev_;
	mutable odb::weak_node<control_unit_n> alg_;

	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int /* file_version */) {
		ar & dev_;
		ar & alg_;
		ar & addr_;
		ar & type_;
		ar & ref_cnt_;
		ar & status_;
		ar & def_value_.data;
		ar & def_value_modified_;
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

	constexpr static std::string_view to_ctype(int type) noexcept;
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
	constexpr static nptype::Type GetClearType(int type) noexcept;

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
	nptype::Type GetClearType() const noexcept;
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
	void DefaultValue_SetValue(bool value) noexcept;
	void DefaultValue_SetValue(u8 value) noexcept;
	void DefaultValue_SetValue(i8 value) noexcept;
	void DefaultValue_SetValue(u16 value) noexcept;
	void DefaultValue_SetValue(i16 value) noexcept;
	void DefaultValue_SetValue(u32 value) noexcept;
	void DefaultValue_SetValue(i32 value) noexcept;
	void DefaultValue_SetValue(f32 value) noexcept;
	DefaultValue DefaultValue_GetValue() const noexcept;
	std::string PrintStatus() const noexcept;
	virtual std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept final {
		return odb::MakeMemento(*this, addr_, type_, ref_cnt_, status_);
	}
	std::string DefaultValue_ToString() const noexcept;
	void DefaultValue_FromString(const std::string& str) ;
	bool DefaultValue_Modified() const;
	void DefaultValue_SetModified(bool modified);
private:
	void m_DefaultValue_Clear(nptype::Type tp) noexcept;
};

} // namespace npsys

#include "variable.inl"

using var_list = std::vector<npsys::variable_n>;
using var_node_ptr_list = std::vector<npsys::variable_n*>;
using var_raw_ptr_list = std::vector<npsys::variable*>;

#endif // !__VARIABLE_H__
