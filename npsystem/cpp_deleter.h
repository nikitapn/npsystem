// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <memory>

class CLine;
class CSFCLine;

struct cpp_line_delete {
	__declspec(noinline) void operator()(CLine*);
};

struct cpp_sfc_line_delete {
	__declspec(noinline) void operator()(CSFCLine*);
};