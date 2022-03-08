// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npsys/variable.h>
#include <npsys/other/write_default_value_request.h>

namespace detail {
class CVariableLoader {
public:
	CVariableLoader(npsys::device_n& device)
		: device_(device) {}
	void Load();
	void Commit();
protected:
	using vs = npsys::variable::Status;
	typedef std::vector<std::unique_ptr<VarWrapper>>::iterator iterator;
	//
	int FillBits(iterator& it, iterator end, WriteDefaultValuesRequest* req);
	void ConstructRequests();
	void CollectDeletedVariables();
	//
	npsys::device_n& device_;
	std::vector<WriteDefaultValuesRequest> m_requests;
	std::vector<std::unique_ptr<VarWrapper>> m_all;
};
} // namespace detail

class CSimpleVariableLoader : public detail::CVariableLoader {
public:
	CSimpleVariableLoader(npsys::device_n& device);
	CSimpleVariableLoader(npsys::device_n& device, var_node_ptr_list& allocated_vars);
};

class CAlgorithmVariableLoader : public detail::CVariableLoader {
public:
	// unload algorihm constructor
	CAlgorithmVariableLoader(
		npsys::device_n& device,
		npsys::fbd_control_unit_n& alg);

	// load algorihm constructor
	CAlgorithmVariableLoader(
		npsys::device_n& device,
		npsys::fbd_control_unit_n& alg,
		var_node_ptr_list& alg_variables,
		var_raw_ptr_list& alg_external_ref_variables);
private:
	npsys::fbd_control_unit_n& alg_;
	std::vector<npsys::variable_n> owner_released_vars_;
	std::vector<npsys::variable_n> new_allocated_vars_;
	void CollectAllOwnerReleasedVariables() noexcept;
};