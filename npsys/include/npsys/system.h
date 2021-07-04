// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "header.h"

namespace npsys {

class CSystem 
	: public odb::common_interface<odb::no_interface>
	, public odb::systree_item<CSystem> {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /*file_version*/) {
		ar & name_;
	}
public:
	CSystem() = default;
	explicit CSystem(std::string name);
};

}
