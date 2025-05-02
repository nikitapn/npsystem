// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "header.h"

namespace npsys {
class CAlgorithmCat 
	: public odb::common_interface<odb::no_interface>
	, public odb::systree_item<CAlgorithmCat> {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int /* file_version */) {
		ar & name_;
		ar & units;
	}
public:
	control_unit_l units;
	CAlgorithmCat() = default;
	CAlgorithmCat(std::string name) 
		: odb::systree_item<CAlgorithmCat>(name) {}
//	virtual void HandleRequest(REQUEST* req) noexcept final;
//	virtual void ConstructMenu(CMenu& menu) noexcept;
protected:
//	virtual void PostDelete() noexcept;
//	virtual void LoadChildsImpl() noexcept;
//	void CreateAlgorithm() noexcept;
//	algorithm_category_n cat_;
};
}; // namespace npsys
