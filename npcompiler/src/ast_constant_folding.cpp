#include "ast.hpp"

// kids don't do this at home, just use llvm

namespace npcompiler::ast {

bool is_algebraic_exp(const AstNode& n) {
	return (n.type >= AstType::Add) && (n.type <= AstType::Uminus);
}

bool is_algebraic_binary_exp(const AstNode& n) {
	return (n.type >= AstType::Add) && (n.type <= AstType::Div);
}



bool is_algebraic_exp_constexpr(const AstNode& n) {
	assert(is_algebraic_exp(n));
	return is_number(n[0]) && is_number(n[1]);
}

bool is_term(const AstNode& n) {
	return (n.type >= AstType::Identifier) && (n.type <= AstType::StringLiteral);
}

bool is_ident(const AstNode& n) {
	return n.type == AstType::Identifier;
}

void perform_expression(AstNode& n) {
	assert(is_number(n[0]) && is_number(n[1]));

	AstNode::number_t result;

	switch (n.type) {
	case AstType::Add:
		result = n[0].as_number() + n[1].as_number();
		break;
	case AstType::Mul:
		result = n[0].as_number() * n[1].as_number();
		break;
	default:
		throw std::runtime_error("perform_expression: compiler error - forbidden type");
	}

	n.delete_children();
	n = result;
}

void perform_unary(const AstType unary_type, AstNode& n) {
	switch (unary_type) {
	case AstType::Uminus:
		if (n.type == AstType::Uminus) {
			n = std::move(n[0]);
		} else if (is_algebraic_binary_exp(n)) {
			perform_unary(unary_type, n[0]);
			if (n.type == AstType::Add)
				perform_unary(unary_type, n[1]);
		} else if (is_number(n)) { 
			n.as_number() = -n.as_number();
		} else if (n.type == AstType::Identifier) {
			n.as_ident().negate ^= 1;
		} else {
			throw std::runtime_error("perform_unary: compiler error - forbidden type");
		}
		break;
	default:
		throw std::runtime_error("perform_unary: compiler error - forbidden type");
	}
}

int priority(AstNode& n) {
	switch (n.type) {
	case AstType::Add: 
		return 0;
	case AstType::Mul: 
	case AstType::Div: 
		return 1;
	default:
		assert(false);
		return -1;
	}
}

bool apply_transform(AstNode& root, size_t root_cv_ix, AstNode& n, AstNode*& exp_ptr) {
	if (priority(root) < priority(n)) return false;

	switch (root.type) {
	case AstType::Add:
	{
		if (is_number(n[0]) || is_number(n[1])) {
			size_t sub_exp_cv_ix = !is_number(n[0]);
			exp_ptr = &n[!sub_exp_cv_ix];
			n.set_child(!sub_exp_cv_ix, &root[root_cv_ix]);
			root.set_child(root_cv_ix, &n);
			return true;
		} else {
			if (is_algebraic_binary_exp(n[0]) || is_algebraic_binary_exp(n[1])) {
				const size_t exp_ix = !is_algebraic_binary_exp(n[0]);
				// try push cv futher down recursively
				return apply_transform(root, root_cv_ix, n[exp_ix], n.child_address_of(exp_ix));
			} else {
				return false;
			}
		}
	}
	case AstType::Mul:
	{
		if (n.type == AstType::Add && (is_term(n[0]) || is_term(n[1]))) {
			assert(&n == &root[!root_cv_ix]);
			auto cv_root = &root[root_cv_ix];
			root.set_child(0, new AstNode(non_term, AstType::Mul, &n[0], cv_root));
			root.set_child(1, new AstNode(non_term, AstType::Mul, &n[1], new AstNode(*cv_root)));
			root.type = AstType::Add;
			delete &n;
			return true;
		} else if (n.type == AstType::Mul && (is_number(n[0]) || is_number(n[1]))) {
			size_t cv_ix = is_number(n[1]);
			exp_ptr = &n[!cv_ix];
			n.set_child(!cv_ix, &root[root_cv_ix]);
			root.set_child(root_cv_ix, &n);
			return true;
		} else if (n.type == AstType::Mul) {
			for (size_t i = 0; i < 2; ++i) {
				if (is_algebraic_exp(n[i]) &&
					apply_transform(root, root_cv_ix, n[i], n.child_address_of(i)))
					return true;
			}
		}
		return false;
	}
	default:
		throw std::runtime_error("apply_transform: compiler error - forbidden type");
	}
}

void print_expr_hr(ast::AstNode& n) {
	std::cerr << n << '\n';
	if (!n.is_non_teminal()) return;
	
	int cur_height = -1;
	bool hchg;
	std::vector<bool> v;
	v.reserve(32);
	for (auto exp : filter_dfs_preorder(n, [&](auto, auto height, bool last) {
		hchg = cur_height != cur_height; 
		cur_height = static_cast<int>(height); 
		if (v.size() < height) v.resize(height);
		v[height - 1] = last;
		return true;
	} )) {
		// "\u2502" │
		// "\u251c" ├
		// "\u2514" └

		for (int i = 0; i < cur_height - 1; ++i) {
			std::cerr << (v[i] ? " " : "\u2502");
		}
		
		if (v[cur_height - 1]) {
			std::cerr << "\u2514";
		} else {
			std::cerr << (hchg ? "\u2502" : "\u251c");
		}
		
		std::cerr << *exp << "\n";
	}
}

void try_rearrange(ast::AstNode& n) {
	assert(!(is_number(n) && is_number(n)));

	if (is_algebraic_exp(n[0]) && is_algebraic_exp(n[1]) && n[0].type == n.type && n[1].type == n.type) {
	//
	//              +
	//         /        \
	//				 +          +
	//       /  \       /   \
	//      a    b     c     d
	//
	//              +
	//            /   \
	//           +     d
	//         /   \
	//        +     c
	//       / \
	//      a   b
	//
		auto d = &n[1][1];
		n[1].set_child(1, &n[0]);
		n.set_child(0, &n[1]);
		n.set_child(1, d);
	}

	else if (is_algebraic_exp(n[0]) && is_algebraic_exp(n[1]) &&  n.type == AstType::Add &&
		(n[0].type == AstType::Add || n[1].type == AstType::Add)) {
	//
	//              +
	//         /        \
	//				 x          x
	//       /  \       /   \
	//      a    b     c     d
	//
	//              +
	//            /   \
	//           +     d
	//         /   \
	//        x     c
	//       / \
	//      a   b
	//
//		std::cerr << "before\n";
//		print_expr_hr(n);
		
		size_t add_ix = n[0].type == AstType::Add ? 0 : 1;

		auto d = &n[add_ix][1];
		n[add_ix].set_child(1, &n[!add_ix]);
		n.set_child(0, &n[add_ix]);
		n.set_child(1, d);

//		std::cerr << "after\n";
//		print_expr_hr(n);
	}

}

template<typename Comp>
std::optional<std::pair<ast::AstNode*, size_t>> find_the_same(ast::AstNode& n, const int top_priority, const Comp& comp) {
	assert(is_algebraic_binary_exp(n));
	
	if (comp(n[0])) {
		return std::make_optional<std::pair<ast::AstNode*, size_t>>(&n, 0);
	} else if (comp(n[1])) {
		return std::make_optional<std::pair<ast::AstNode*, size_t>>(&n, 1);
	}

	if (priority(n) != top_priority) return std::nullopt;

	if (is_algebraic_binary_exp(n[0])) {
		return find_the_same(n[0], top_priority, comp);
	} else if (is_algebraic_binary_exp(n[1])) {
		return find_the_same(n[1], top_priority, comp);
	} 
	
	return std::nullopt;
}

bool try_commutative_number(ast::AstNode& n, size_t ix) {
	auto founded = find_the_same(n[!ix], priority(n),
		[](const ast::AstNode& nt) { 
			return is_number(nt); 
		}
	);

	if (founded) {
		auto& nf = *(*founded).first;
		auto fix = (*founded).second;
		if (n.type == nf.type) {
			//              +
			//            /   \
			//           +     n
			//         /   \
			//        +     y
			//       / \
			//      n   x
			//
				//std::cerr << "before\n";
				//print_expr_hr(n);
			auto tmp = &n[ix];
			n.set_child(ix, &nf[!fix]);
			nf.set_child(!fix, tmp);
			//std::cerr << "after\n";
			//print_expr_hr(n);

			return true;
		}
	}


	return false;
}

bool try_commutative_ident(ast::AstNode& n, size_t ix) {
	//print_expr_hr(n);
	auto founded = find_the_same(n[!ix], priority(n),
		[&same_as = n[ix]](const ast::AstNode& nt) { 
			return is_ident(nt) && nt.as_ident().name == same_as.as_ident().name; 
		}
	);
	if (founded) {
		auto& nf = *(*founded).first;
		auto fix = (*founded).second;


		if (nf.type == AstType::Mul) {
	//              +
	//            /   \
	//           +     x
	//         /   \
	//        *     y
	//       / \
	//      5   x
	//
			if (is_number(nf[!fix])) {
				delete& n[ix];
				n = n[!ix];
				nf[!fix].as_number() += 1;
				return true;
			} else {
			//	print_expr_hr(n);
			//	print_expr_hr(nf);
				//assert(is_ident(nf[!fix]));
	//              +
	//            /   \
	//           +     x
	//         /   \
	//        *     y
	//       / \
	//      z   x
	//
	//	do nothing...
			}
		} else if (nf.type == AstType::Add) {
	//              +
	//            /   \
	//           +     x
	//         /   \
	//        +     y
	//       / \
	//      5   x
	// or
	//              +
	//            /   \
	//           +     x
	//         /   \
	//        +     y
	//       / \
	//      z   x
			auto tmp = &n[ix];
			n.set_child(ix, &nf[!fix]);
			nf.set_child(!fix, tmp);
			nf.type = AstType::Mul;
			nf[!fix] = {2};
			return true;
		} else if (nf.type == AstType::Div) {

		} else {
			assert(false);
		}

		//print_expr_hr(*founded.value().first);

		
	}

	return false;
}

// 0 - exp is a constant value
// 1 - exp is an identifier
// 2 - exp has an identifier
// 4 - exp has a constant
// 8 - both exp are different identifiers

int constant_folding(ast::AstNode& n) {
	if (is_number(n)) return 0;
	if (is_ident(n)) return 1;

	assert(is_algebraic_exp(n));

	if (n.type == AstType::Uminus) {
		auto exp_ptr = &n[0];
		n = std::move(*exp_ptr);
		delete exp_ptr;
		perform_unary(AstType::Uminus, n);
		return constant_folding(n);
	}

	if (is_number(n[0]) && is_number(n[1])) {
		perform_expression(n);
		return 0;
	}

	auto r0 = constant_folding(n[0]),
		r1 = constant_folding(n[1]);

	if (r0 != 0 && r1 != 0) {
		try_rearrange(n);
	}

	r0 = constant_folding(n[0]);
	r1 = constant_folding(n[1]);

	switch (r0 | (r1 << 16)) {
	case (0 | 0 << 16): // both are constants
		perform_expression(n);
		return 0;
	case (1 | 0 << 16):
	case (0 | 1 << 16): // one of the expressions is a constant value expression and the other is an identifier
		return 2;
	case (1 | 1 << 16): // both expressions are identifiers
	{
		if (n[0].as_ident().name == n[1].as_ident().name) {
			switch (n.type) {
			case AstType::Add:
				n.type = AstType::Mul;
				n[0] = {2};
				return 2;
			case AstType::Div:
				n.delete_children();
				n = {1};
				return 0;
			};
		} else {
			return 8;
		}
	}
	case (0 | 2 << 16):
	case (2 | 0 << 16): // one of the expressions is a constant and the other has an identifier
		// do nothing for now
		//print_expr_hr(n);
		return 4;
	case (1 | 2 << 16):
	case (2 | 1 << 16): // one of the expressions is an identifier and the other has an identifier
	{
		const size_t ident_ix = (r0 == 1 ? 0 : 1);
		const size_t other_ident_ix = (is_ident(n[!ident_ix][0]) ? 0 : 1);
		if (n[ident_ix].as_ident().name == n[!ident_ix][other_ident_ix].as_ident().name) {
			if (is_number(n[!ident_ix][!other_ident_ix])) {
				if (n.type == AstType::Add) {
					if (n[!ident_ix].type == AstType::Mul) {
						delete& n[!ident_ix][other_ident_ix];
						auto old = &n[!ident_ix];
						n.set_child(!ident_ix, &n[!ident_ix][!other_ident_ix]);
						delete old;
						n.type = AstType::Mul;
						n[!ident_ix].as_number() += 1;
						return 2;
					}
				} else if (n.type == AstType::Mul) {
					if (n[!ident_ix].type == AstType::Add) {
	//              *                    +
	//            /   \               /     \
	//           +     x     ->      *       *
	//         /   \                / \     / \
	//        n     x              x   x   x   n
						auto x = &n[ident_ix];
						auto x1 = &n[!ident_ix];
						n.set_child(1, new AstNode(non_term, AstType::Mul, x, new AstNode(x->as_ident())));
						n.set_child(0, x1);
						n[0].type = AstType::Mul;
						n.type = AstType::Add;
						return -1;
					}
				}
			}
		}
	} 
	return -1;
	case (0 | 4 << 16):
	case (4 | 0 << 16): // one of the expressions is a constant and the other has a constant
	{
		const size_t constant_ix = (is_number(n[0]) ? 0 : 1);
		if (apply_transform(n, constant_ix, n[!constant_ix], n.child_address_of(!constant_ix))) {
			return constant_folding(n);
		} else {
			return -1;
		}
	}
	case (2 | 2 << 16): // both expressions have an identifier and a constant
	{
		size_t ident_1 = is_ident(n[0][0]) ? 0 : 1;
		size_t ident_2 = is_ident(n[1][0]) ? 0 : 1;

		if (n[0][ident_1].as_ident().name != n[0][ident_2].as_ident().name) return -1;

		if (n[0].type == AstType::Mul && n[1].type == AstType::Mul) {
		// 3*x + 5*x;
			if (n.type == AstType::Add) {
				//std::cerr << "\n\n<\n";
				//print_expr_hr(n);
				
				n[0][!ident_1].as_number() += n[1][!ident_2].as_number();
				delete &n[1][ident_2];
				delete &n[1];
				auto& tmp = n[0];
				n.type = AstType::Mul;
				n.set_child(0, &tmp[0]);
				n.set_child(1, &tmp[1]);
				delete &tmp;


				//print_expr_hr(n);
				//std::cerr << ">\n\n";

				return 2;
			} else if (n.type == AstType::Mul) {

			} else if (n.type == AstType::Div) {

			} else {
				assert(false);
			}
		}
		
		return -1;

		// 2+x + 4*x;

		break;
	}
	default:
		if (is_number(n[0]) && try_commutative_number(n, 0)) { return constant_folding(n); }
		else if (is_number(n[1]) && try_commutative_number(n, 1)) { return constant_folding(n); }
		else if (is_ident(n[0]) && try_commutative_ident(n, 0)) { return constant_folding(n); }
		else if (is_ident(n[1]) && try_commutative_ident(n, 1)) { return constant_folding(n); }
		else return -1;
	};

	throw std::runtime_error("constant_folding: compiler error"); 
}

void do_constant_folding(ParserContext& ctx, AstNode& root) {
	auto exp_to_simplify = filter_branch_discard_dfs_preorder(root, [](AstNode& n, size_t depth) {
		return is_algebraic_exp(n);
	});

	for (auto n : exp_to_simplify) {
		std::cerr << "exp_to_simplify:\n";
		print_expr_hr(*n);
		constant_folding(*n);
		std::cerr << '\n' << "simplified:\n";
		print_expr_hr(*n);
		std::cerr << '\n';
	}
}

} // namespace ast