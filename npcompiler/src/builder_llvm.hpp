#pragma once

#include "ast.hpp"

namespace npcompiler {

class BuilderLLVM {
	class BuilderLLVMImpl* impl_;
public:
	void emit(ast::AstNode& n);
	void print_debug();
	void avr_create_object();

	BuilderLLVM(std::string_view module_name);
	~BuilderLLVM();
};

} // namespace npcompiler
