// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npsys/header.h>

class CDynamicLinker {
public:
	enum Result {
		R_CRITICAL_ERROR,
		R_ERROR,
		E_CANCELED,
		R_OK
	};
	virtual bool AssignControlUnit(npsys::control_unit_n& alg) = 0;
	virtual Result UploadFBDUnit(npsys::fbd_control_unit_n& alg) = 0;
	virtual Result UnloadFBDUnit(npsys::fbd_control_unit_n& alg) = 0;
	virtual bool AssignI2CModule(const npsys::i2c_module_n& mod, npsys::i2c_assigned_module_n& assigned) = 0;
	virtual Result UploadI2CModule(npsys::i2c_assigned_module_n& mod) = 0;
	virtual bool UnloadModule(npsys::i2c_assigned_module_n& mod) = 0;
	virtual ~CDynamicLinker() = default;
};