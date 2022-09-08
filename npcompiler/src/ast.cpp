#include "ast.hpp"

namespace npcompiler::ast {
	
AstNode* ParserContext::create_binary_op(AstType type, AstNode* lhs, AstNode* rhs) {
	std::cerr << "binop: " << *lhs << " , " << *rhs << "\n";  
	auto t1 = lhs->get_expression_type(), t2 = rhs->get_expression_type();
	int cres = npsys::nptype::compare(t1, t2);
	if (cres == 0) return new AstNode(type, lhs, rhs);

	auto cast_to = (cres == 1 ? t1 : t2);
	auto [n1, n2] = (cast_to == t1 ? std::make_tuple(lhs, rhs) : std::make_tuple(rhs, lhs));

	auto n = new AstNode(
		type,
		new AstNode(std::forward_as_tuple(AstType::Cast, CastTo{ cast_to }), n2),
		n1
	);

	return n;
}

AstNode* ParserContext::create_assignment(AstNode* lhs, AstNode* rhs) {
	auto src = lhs->get_expression_type(), dest = rhs->get_expression_type();
	int cres = npsys::nptype::compare(src, dest);
	if (src == dest) new AstNode(AstType::Assignment, lhs, rhs);

	//std::cerr << "Assignment: " << npsys::variable::to_ctype(src) << " := " << npsys::variable::to_ctype(dest) << '\n';

	if (src == -1) {
		std::cout << "assignment may result loss of data\n";
	}

	return new AstNode(
		AstType::Assignment,
		lhs,
		new AstNode(std::forward_as_tuple(AstType::Cast, CastTo{ src }), rhs)
	);
}


} // namespace npcompiler::ast