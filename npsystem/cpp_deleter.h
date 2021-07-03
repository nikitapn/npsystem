#pragma once

#include <memory>

class CLine;
class Command;

struct cpp_command_delete {
	__declspec(noinline) void operator()(Command*);
};

struct cpp_line_delete {
	__declspec(noinline) void operator()(CLine*);
};