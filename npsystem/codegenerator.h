// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npsys/variable.h>

class CCodeGenerator {
	using variable = npsys::variable;
protected:
	std::stringstream m_code;
public:
	virtual void Reset() = 0;
	#define AAF(x) virtual void Generate(x*) = 0;
		ALPHA_BLOCKS()
	#undef AAF
};

class CTypeDeterminant : public CCodeGenerator {
	int get_output_1(int t1, int t2);
public:
	virtual void Reset();
	#define AAF(x) virtual void Generate(x*) ;
		ALPHA_BLOCKS()
	#undef AAF
};
