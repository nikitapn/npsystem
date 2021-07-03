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
