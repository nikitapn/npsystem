// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "mctypes.h"
#include <nplib/utils/hexparser.h>
#include <sim/import_export.h>

class HexParserFlash : public HexParser {
public:
	HexParserFlash(std::string_view filename) 
		: HexParser(filename) {}

	SIM_IMPORT_EXPORT
	void Read(Flash& flash);
};
