// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "network_ext.h"
#include "variableloader.h"


// By Default it sets zero into values cells, quality is set invalid also 
int VarWrapper::InsertVariable(WriteDefaultValuesRequest* request) const noexcept {
	int size_with_quality = GetSizeWithQuality();

	if (size_with_quality + request->m_len >= WriteDefaultValuesRequest::MAX_DATA_LEN) return 1;

	int addr = GetAddr();
	int ix = addr - request->m_offset;

	if (request->m_prOffset != addr) {
		request->m_len += size_with_quality;
	}

	request->m_prOffset = addr;

	return 0;
}

int VarWrapper1::InsertVariable(WriteDefaultValuesRequest* request) const noexcept {
	int size = var_->GetSize();

	if (size + request->m_len >= WriteDefaultValuesRequest::MAX_DATA_LEN) return 1;

	int ix = var_->GetAddr() - request->m_offset;

	if (var_->IsBit()) {
		bool b = var_->DefaultValue_GetValue()._b;
		request->m_data[ix] |= b << var_->GetBit();
		if (var_->IsQuality()) {
			request->m_data[ix] |= 2 << var_->GetBit();
		}
	} else {
		auto value = var_->DefaultValue_GetValue();
		for (int i = 0; i < size; ++i) {
			request->m_data[ix + i] = value.data[i];
		}
		if (var_->IsQuality()) {
			request->m_data[ix + size] = VQ_GOOD;
		}
	}

	if (request->m_prOffset != var_->GetAddr()) {
		request->m_len += size ? size + var_->IsQuality() : 1;
	}
	request->m_prOffset = var_->GetAddr();

	return 0;
}

namespace detail {

void CVariableLoader::CollectDeletedVariables() {
	// collect released modules values and other stuff
	device_->vars_owner_released_.fetch_all_nodes();
	for (auto& v : device_->vars_owner_released_) {
		if (v->GetStatus() == npsys::variable::Status::to_remove &&
			v->GetAlg().is_invalid_id()) {
			m_all.push_back(std::make_unique<VarWrapper3>(v.get()));
		}
	}
}

void CVariableLoader::ConstructRequests() {
	std::sort(m_all.begin(), m_all.end(), [](const auto& p1, const auto& p2) {
		return p1->GetAddr() < p2->GetAddr();
	});

	std::cout << ">Variables to load:\n";
	for (auto& vw : m_all) {
		auto var = vw->var();
		if (var->DefaultValue_Modified()) {
			std::cout << '\t' << *var << " def_value: " << var->DefaultValue_ToString() << '\n';
		}
	}

	auto from = m_all.begin();
	const auto end = m_all.end();
	
	for (;from != end;) {
		auto var = (*from).get();
		WriteDefaultValuesRequest* current_req = nullptr;
		if (var->IsBit()) {
			WriteDefaultValuesRequest req(var->GetAddr());
			if (FillBits(from, end, &req)) {
				m_requests.push_back(req);
				current_req = &m_requests.back();
			} else {
				from++;
				continue;
			}
		} else {
			if (!var->IsModified()) {
				from++;
				continue;
			}
			m_requests.push_back(WriteDefaultValuesRequest(var));
			current_req = &m_requests.back();
		}

		iterator prev = from;
		iterator next = std::next(from);
		for (; next != end; next++) {
			if (!current_req->IsByteAvailable()) break;
			if (((*prev)->GetAddr() + (*prev)->GetSizeWithQuality()) != (*next)->GetAddr()) break;
			
			if ((*next)->IsBit()) {
				if (!FillBits(next, end, current_req)) break;
			} else {
				if (!(*next)->IsModified()) break;
				if ((*next)->InsertVariable(current_req) == 1) break;
			}
			prev = next;
		}

		from = next;
	}
}

int CVariableLoader::FillBits(iterator& it, iterator end, WriteDefaultValuesRequest* request) {
	ASSERT((*it)->IsBit());

	auto needs_update = false;
	uint16_t addr = (*it)->GetAddr();
	boost::container::small_vector<VarWrapper*, 8> bits;

	for (;;) {
		needs_update |= (*it)->IsModified();
		bits.push_back((*it).get());

		auto next = std::next(it);
		if (next == end || addr != (*next)->GetAddr()) break;

		it = next;
	}

	if (!needs_update) return 0;

	for (auto& i : bits) i->InsertVariable(request);

	return 1;
}

void CVariableLoader::Load() {
	if (m_requests.empty()) return;

	size_t rn = 0;
	std::cout << ">Ram Update Started:\n"sv;
	for (auto const& r : m_requests) {
		std::cout << "\tR #"sv << rn++ << ": o. "sv << to_hex(r.GetOffset()) << " d. "sv;
		int len = r.GetLength();
		for (int i = 0; i < len - 1; ++i) {
			std::cout << to_hex(r.GetData()[i]) << ", "sv;
		}
		std::cout << to_hex(r.GetData()[len - 1]) << ";\n"sv;
	}
	device_->WriteDefaultValues(m_requests);
}

void CVariableLoader::Commit() {
	if (m_requests.empty()) return;
	
	for (auto& var : m_all) var->Commit();

	using vs = npsys::variable::Status;
	
	auto it = device_->vars_owner_released_.begin();
	auto end = device_->vars_owner_released_.end();

	while (it != end) {
		if ((*it)->GetStatus() == vs::to_remove) {
			if ((*it)->RefCount() != 0) {
				(*it)->SetStatus(vs::removed_refcnt_not_null);
				(*it).store();
			} else {
				it = device_->vars_owner_released_.erase(it, true);
				end = device_->vars_owner_released_.end();
				continue;
			}
		}
		it++;
	}
	device_->vars_owner_released_.store();
}

} // namespace detail


CSimpleVariableLoader::CSimpleVariableLoader(
	npsys::device_n& device,
	var_node_ptr_list& allocated_vars)
	: CVariableLoader(device) {
	for (auto var : allocated_vars) {
		ASSERT(!(*var)->IsIO());
		if ((*var)->GetClearType() != npsys::nptype::NPT_UNDEFINE) {
			m_all.push_back(std::make_unique<VarWrapper1>((*var).get()));
		}
	}
	CollectDeletedVariables();
	ConstructRequests();
}

CSimpleVariableLoader::CSimpleVariableLoader(npsys::device_n& device)
	: CVariableLoader(device) {
	CollectDeletedVariables();
	ConstructRequests();
}

// CAlgorithmVariableLoader
#include "control_unit_ext.h"
#include "assignedalgorithm.h"

CAlgorithmVariableLoader::CAlgorithmVariableLoader(
	npsys::device_n& device,
	npsys::fbd_control_unit_n& alg)
	: CVariableLoader(device)
	, alg_(alg)
{
	CollectAllOwnerReleasedVariables();
	ConstructRequests();
}

void CAlgorithmVariableLoader::CollectAllOwnerReleasedVariables() noexcept {
	device_->vars_owner_released_.fetch_all_nodes();
	for (auto& var : device_->vars_owner_released_) {
		if (var->GetAlg().id() == alg_.id() &&
			var->GetStatus() == vs::to_remove) {
			owner_released_vars_.push_back(var);
		}
	}
	for (auto& var : owner_released_vars_) {
		assert(!var->IsIO());
		m_all.push_back(std::make_unique<VarWrapper3>(var.get()));
	}
}

CAlgorithmVariableLoader::CAlgorithmVariableLoader(
	npsys::device_n& device,
	npsys::fbd_control_unit_n& alg,
	var_node_ptr_list& algVars,
	var_raw_ptr_list& algRefs)
	: CVariableLoader(device)
	, alg_(alg)
{
	// collect all variables (with modified defaul value, all new allocated, previously loaded also)
	for (auto var_ptr : algVars) {
		auto var = *var_ptr;
		ASSERT(!var->IsIO());
		if (var->GetClearType() != npsys::nptype::NPT_UNDEFINE) {
			m_all.push_back(std::make_unique<VarWrapper1>(var.get()));
		}
	}

	// only new allocated ext ref variables
	for (auto& var : algRefs) {
		ASSERT(!var->IsIO());
		if (var->GetClearType() != npsys::nptype::NPT_UNDEFINE && var->GetStatus() != vs::good) {
			m_all.push_back(std::make_unique<VarWrapper2>(var));
		}
	}

	CollectAllOwnerReleasedVariables();
	CollectDeletedVariables();
	ConstructRequests();
}