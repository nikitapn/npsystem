#include "ast.hpp"

namespace npcompiler::ast {
	
AstNode* ParserContext::create_binary_op(AstType type, AstNode* lhs, AstNode* rhs) {
	auto t1 = lhs->get_expression_type(), t2 = rhs->get_expression_type();
	if (t1 == t2) return new AstNode(type, lhs, rhs);

	auto cast_to = std::max(t1, t2);
	auto [n1, n2] = cast_to == t1 ? std::make_tuple(lhs, rhs) : std::make_tuple(rhs, lhs);

	auto n = new AstNode(
		type,
		new AstNode(std::forward_as_tuple(AstType::Cast, CastTo{ cast_to }), n1),
		n2
	);

	return n;
}

AstNode* ParserContext::create_assignment(AstNode* lhs, AstNode* rhs) {
	auto src = lhs->get_expression_type(), dest = rhs->get_expression_type();
	if (src == dest) new AstNode(AstType::Assignment, lhs, rhs);

	//std::cerr << "Assignment: " << npsys::variable::to_ctype(src) << " := " << npsys::variable::to_ctype(dest) << '\n';

	if (src > dest) {
		std::cout << "assignment may result loss of data\n";
	}

	return new AstNode(
		AstType::Assignment,
		lhs,
		new AstNode(std::forward_as_tuple(AstType::Cast, CastTo{ src }), rhs)
	);
}

} // namespace npcompiler::ast