#pragma once

#include "ast.hpp"

namespace npcompiler {

class BuilderLLVM {
	class BuilderLLVMImpl* impl_;
public:
	void emit(ast::AstNode& n);

	BuilderLLVM();
	~BuilderLLVM();
};

} // namespace npcompiler
