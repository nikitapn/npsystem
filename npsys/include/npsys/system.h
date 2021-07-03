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
