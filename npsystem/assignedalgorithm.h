#pragma once

#include "algext.h"
#include <npsys/other/remote.h>

class CAlgorithmVariableLoader;

namespace npsys {
class CAssignedAlgorithm 
	: public odb::common_interface<CAssignedAlgorithm>
	, public IUploadable {
	friend variable; // access to vars_released_
	friend CAlgorithmVariableLoader;// access to vars_released_
	friend boost::serialization::access;
	template<class Archive>
	void save(Archive& ar, const unsigned int /*file_version*/) const {}
	template<class Archive>
	void load(Archive& ar, const unsigned int /*file_version*/) {
		ASSERT_FETCH(alg);
	}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & dev;
		ar & alg;
		ar & remote_var_released_;
		ar & reference_released_;
		boost::serialization::split_member(ar, *this, file_version);
	}
public:
	algorithm_n alg;
	odb::weak_node<device_n> dev;
	inline static std::string error = "???";
	CTreeItemAbstract* item = nullptr;
protected:
	std::vector<npsys::remote::LinkData> remote_var_released_;
	odb::node_list<variable_n> reference_released_;
public:
	CAssignedAlgorithm() = default;
	CAssignedAlgorithm(algorithm_n& _alg, device_n& _dev) 
		: alg(_alg)
		, dev(_dev)
	{
		ASSERT(_alg.loaded());
	}
	const std::string& get_name() const noexcept { 
		return alg.loaded() ? alg->get_name() : error;
	}
	auto const& references_released() const noexcept { return reference_released_; }
	void AddVariableReferenceToRemoveList(npsys::variable_n& ref) noexcept;
	void AddExternalReferenceToRemoveList(CExternalReference* ref) noexcept;
	virtual void ReleaseRemovedReferences() noexcept;
};
} // namespace npsys