#pragma once

namespace npcompiler {
namespace ast {
class AstNode;
}

class Typer {
public:
	void emit(ast::AstNode& n);
};

} // namespace npcompiler
