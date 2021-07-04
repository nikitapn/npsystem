// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "assignedalgorithm.h"
#include "network_ext.h"
#include "block.h"

namespace npsys {
void CAssignedAlgorithm::AddVariableReferenceToRemoveList(npsys::variable_n& ref) noexcept {
	reference_released_.add_node(ref);
}

void CAssignedAlgorithm::AddExternalReferenceToRemoveList(CExternalReference* ext) noexcept {
	ASSERT(ext->GetPlace() == CExternalReference::OTHER_DEVICE);
	const auto var = ext->variable_;
	const auto ref_var = ext->link_.loaded_variable.fetch();
	remote_var_released_.push_back(
		{ var, ref_var, ext->GetType(), var->IsBit() }
	);
}

void CAssignedAlgorithm::ReleaseRemovedReferences() noexcept {
	auto ReleaseRef = [](npsys::variable_n& var) {
		using vs = variable::Status;

		ASSERT(var->ref_cnt_);
		var->ref_cnt_--;

		if (!var->IsIO() &&
			var->ref_cnt_ == 0 &&
			var->status_ == vs::removed_refcnt_not_null) {
			auto device = var->dev_.fetch();
			if (device.loaded()) {
				device->vars_owner_released_.erase(var, true);
				device->vars_owner_released_.store();
			}
		} else {
			var.store();
		}
	};

	reference_released_.fetch_all_nodes();
	for (auto& var : reference_released_) {
		ReleaseRef(var);
	}
	reference_released_.clear_list();

	auto device = dev.fetch();
	for (auto& link : remote_var_released_) {
		if (!link.var.is_invalid_node()) {
			ASSERT_FETCH(link.var);
			device->DeleteLink(link.type, link.var.id(), link.var->IsBit());
		}
		ASSERT_FETCH(link.ref_var);
		ReleaseRef(link.ref_var);
	}
	remote_var_released_.clear();
}
} // namespace npsys
