// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "vartype.h"

// COneVariable

void COneVariable::FillArray(var_list& variables) noexcept {
	variables.push_back(variable_);
}

void COneVariable::FillArray(var_node_ptr_list& variables) noexcept {
	variables.push_back(&variable_);
}

void COneVariable::Alloc(CMemoryManager& mm) noexcept {
	mm.AllocateMemory(this);
}

void COneVariable::Clone(bool store) noexcept {
	variable_ = odb::create_node<npsys::variable_n>(*variable_.get());
	if (store) variable_.store();
}

void COneVariable::ReleaseMemory() noexcept {
	if (variable_.loaded()) {
		variable_->OwnerRelease(nullptr);
	}
}

// CVarContainer

void CVarContainer::FillArray(var_list& variables) noexcept {
	for (auto& v : variables_) {
		variables.push_back(v);
	}
}

void CVarContainer::FillArray(var_node_ptr_list& variables) noexcept {
	for (auto& v : variables_) {
		variables.push_back(&v);
	}
}

void CVarContainer::Alloc(CMemoryManager& mm) noexcept {
	mm.AllocateMemory(this);
}

void CVarContainer::Clone(bool store) noexcept {
	for (auto& var : variables_) {
		var = odb::create_node<npsys::variable_n>(*var.get());
		if (store) var.store();
	}
}

void CVarContainer::ReleaseMemory() noexcept {
	for (auto& var : variables_) {
		if (var.loaded()) {
			var->OwnerRelease(nullptr);
		}
	}
	variables_.clear();
}

void CVarContainer::ShrinkMemory(size_t new_size) noexcept {
	ASSERT(new_size < variables_.size());

	auto from = variables_.begin();
	std::advance(from, new_size);

	for (auto it = from; it != variables_.end(); it++) {
		(*it)->OwnerRelease(nullptr);
	}
	
	variables_.erase(from, variables_.end());
}


int CVarContainer::VarByteSize() const noexcept {
	int sum = 0;
	for (auto& i : variables_) {
		sum += i->GetSize() + i->IsQuality();
	}
	return sum;
}