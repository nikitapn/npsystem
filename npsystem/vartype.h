// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "memorymanager.h"

class CVariable {
	friend class CReleaseVariables;
public:
	virtual void FillArray(var_list& variables) noexcept = 0;
	virtual void FillArray(var_node_ptr_list& variables) noexcept = 0;
	virtual void Alloc(CMemoryManager& mm) noexcept = 0;
	virtual void Clone(bool store) noexcept = 0;
protected:
	virtual void ReleaseMemory() noexcept = 0;
};

class COneVariable : public CVariable {
	friend class CMemoryManager;
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int/* version*/) const {
		ASSERT(variable_.is_invalid_node() == false);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int/* version*/) {
		if (!variable_.fetch()) {
			std::cout << "ERROR: variable wasn't found id = " << variable_.id() << std::endl;
		}
	}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & variable_;
		boost::serialization::split_member(ar, *this, file_version);
	}
public:
	virtual void FillArray(var_list& variables) noexcept;
	virtual void FillArray(var_node_ptr_list& variables) noexcept;
	virtual void Alloc(CMemoryManager& mm) noexcept;
	virtual void Clone(bool store) noexcept;
protected:
	virtual void ReleaseMemory() noexcept;
	npsys::variable_n variable_;
};

class CVarContainer : public CVariable {
	friend class CMemoryManager;
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int/* version*/) const {
		for (auto& v : variables_) ASSERT(v.is_invalid_node() == false);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int/* version*/) {
		for (auto& var : variables_) {
			if (!var.fetch()) {
				std::cout << "ERROR: variable wasn't found id = " << var.id() << std::endl;
			}
		}
	}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & variables_;
		boost::serialization::split_member(ar, *this, file_version);
	}
protected:
	virtual void ReleaseMemory() noexcept;
	void ShrinkMemory(size_t new_size) noexcept;
	//
	std::vector<npsys::variable_n> variables_;
public:
	void AddVariable(npsys::variable_n& var) noexcept { variables_.push_back(var); }
	void AddVariable(npsys::variable_n&& var) noexcept { variables_.push_back(std::move(var)); }
	uint16_t GetOrigin() const noexcept { return Empty() ? 0xffff : variables_[0]->GetAddr(); }
	bool Empty() const noexcept { return variables_.empty(); }
	int VarByteSize() const noexcept;
	size_t VarCount() const noexcept { return variables_.size(); }
	virtual void FillArray(var_list& variables) noexcept;
	virtual void FillArray(var_node_ptr_list& variables) noexcept;
	virtual void Alloc(CMemoryManager& mm) noexcept;
	virtual void Clone(bool store) noexcept;
};

