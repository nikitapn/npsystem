#include "stdafx.h"
#include <npcompiler/npcompiler.hpp>

int test_compiler() {
	try {
		npcompiler::Compilation compilation(std::filesystem::path("../npcompiler/test/example0.txt"));
		return !(compilation.compile() == true);
	} catch (std::exception& ex) {
		std::cerr << ex.what();
		return -1;
	}
}