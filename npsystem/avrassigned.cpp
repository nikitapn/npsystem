// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "libfunction.h"
#include "avrassigned.h"
#include "block.h"
#include <npsys/variable.h>
#include "io_avr.h"

BOOST_CLASS_EXPORT_GUID(npsys::CAVRAssignedAlgorithm, "avr_assigned_alg");

namespace npsys {

// CAVRFlashObject
void CAVRFlashObject::SetRange(int begin, int size_b, int pagesize) {
	size_b_ = size_b;
	start_ = begin / pagesize;
	end_ = (begin + size_b) / pagesize;
}

// CAVRAssignedObjectFile

bool CAVRAssignedObjectFile::SetFunaddr(const std::string& fun_name, int addr) {
	auto begin = expf.begin(), end = expf.end();
	auto it = std::find_if(begin, end, [&fun_name](LibFunction& f) {
		return (f.name == fun_name); });
	if (it == end) return false;
	(*it).addr = addr;
	return true;
}

bool CAVRAssignedObjectFile::CheckFunctions(std::string& error) {
	for (auto& f : expf) {
		if (!f.IsValid()) {
			error = f.name;
			return false;
		}
	}
	return true;
}

std::string CAVRAssignedObjectFile::DefSym() {
	std::string def;
	for (auto& f : expf) {
		std::string str;
		std::stringstream ss;
		ss << std::hex << f.addr;
		ss >> str;
		def += ",--defsym," + f.name + "=0x" + str;
	}
	return def;
}

} // namespace npsys