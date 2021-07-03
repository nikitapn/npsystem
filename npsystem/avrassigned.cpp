#include "stdafx.h"
#include "libfunction.h"
#include "avrassigned.h"
#include "block.h"
#include <npsys/variable.h>
#include "io_avr.h"

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
		ss << std::hex << f.addr; // в GCC в байтах
		ss >> str;
		def += ",--defsym," + f.name + "=0x" + str;
	}
	return def;
}

} // namespace npsys