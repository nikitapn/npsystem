// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npsys/header.h>

class Configurator {
	npsys::controller_n ctrl_;
public:
	void UploadAllAlgoritms() noexcept;
	
	Configurator(npsys::controller_n& ctrl)
		: ctrl_(ctrl)
	{
	}
};