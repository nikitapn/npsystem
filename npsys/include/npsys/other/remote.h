#pragma once

#include <npsys/variable.h>

namespace npsys {
namespace remote {

enum ExtLinkType {
	Read = 1,
	Write = 0,
};

struct DeviceLinks {
	// variable -> remote variable
	using link_list = std::vector<std::pair<variable_n, variable_n>>;

	link_list lbit;
	link_list lbyte;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & lbit;
		ar & lbyte;
	}
	void load_nodes() {
		for (auto& n : lbit) {
			n.first.fetch();
			n.second.fetch();
		}
		for (auto& n : lbyte) {
			n.first.fetch();
			n.second.fetch();
		}
	}
};

struct LinkData {
	variable_n var;
	variable_n ref_var;
	ExtLinkType type;
	bool bit;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & var;
		ar & ref_var;
		ar & type;
		ar & bit;
	}
};

}} // namespace npsys::remote