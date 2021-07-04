// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "header.h"

namespace npsys {
class CAlgorithms : public odb::common_interface<odb::no_interface> {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & alg_cats;
	}
public:
	algorithm_category_l alg_cats;
	
	void CreateAlgorithmCat(std::string&& name) noexcept;
	
#ifdef _CONFIGURATOR_
//	virtual void HandleRequest(REQUEST* req) noexcept final;
//	virtual void ConstructMenu(CMenu* menu) noexcept;
#endif
};
} // namespace npsys