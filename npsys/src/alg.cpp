// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <npsys/control_unit.h>
#include <npsys/algcat.h>

namespace npsys {

// CControlUnit
void CControlUnit::SetScanPeriodMs(uint16_t _scan_period) {
	if (status_ == Status::status_loaded) {
		status_ = Status::status_not_actual;
	}
	npsys::access::rw(scan_period) = _scan_period;
}

// CAlgorithms
void CAlgorithms::CreateAlgorithmCat(std::string&& /* name */) noexcept {
	
}

} // namespace npsys
