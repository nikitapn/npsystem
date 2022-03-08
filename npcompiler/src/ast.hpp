// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#ifndef AST_HPP__
#define AST_HPP__

#include <iostream>
#include <vector>
#include <stack>
#include <array>
#include <string>
#include <string_view>
#include <cassert>
#include <variant>
#include <queue>
#include <memory>
#include <variant>
#include <functional>
#include <boost/pool/pool.hpp>
#include <optional>
#include <coroutine>
#include <experimental/generator>

using std::experimental::generator;

template<class... Ts> struct overloaded : Ts... {};
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#define AST_NODE_TYPES() \
	AST_TYPE(Script), \
	AST_TYPE(StmtList), \
	AST_TYPE(Identifier), \
	AST_TYPE(Number), \
	AST_TYPE(StringLiteral), \
	AST_TYPE(VarDeclSeq), \
	AST_TYPE(Assignment), \
	AST_TYPE(If), \
	AST_TYPE(Add), \
	AST_TYPE(Mul), \
	AST_TYPE(Div), \
	AST_TYPE(Uminus)

namespace llvm {
class Value;
}

namespace npcompiler::ast {

using discrete = bool;
using u8  = unsigned char;
using i8  = signed char;
using u16 = unsigned short;
using i16 = signed short;
using u32 = unsigned int;
using i32 = signed int;
using float32 = float;

using namespace std::string_view_literals;

#define AST_TYPE(x) x
enum class AstType : u32 {
	AST_NODE_TYPES(),
	_size
};
#undef AST_TYPE

static constexpr size_t ast_nodes_size = static_cast<size_t>(AstType::_size);

#define AST_TYPE(x) #x ## sv
static constexpr std::string_view a_node_type_str[] = { AST_NODE_TYPES() };
#undef AST_TYPE


enum class IdentType {
	Variable, Function
};

struct Identifier {
	IdentType type;
	std::string_view name;
};

struct Identifier0 : Identifier {
	bool negate;
};

struct AstNode;

class ParserContext {
	std::vector<std::pair<std::string_view, Identifier>> symbols_;
	std::vector<size_t> scope_;
public:
	void push_scope() {
		scope_.emplace_back();
	}

	void pop_scope() {
		assert(scope_.empty() == false);
		symbols_.erase(symbols_.end() - scope_.back(), symbols_.end());
		scope_.pop_back();
	}

	Identifier& ident_get(std::string_view str) {
		if (auto it = std::find_if(symbols_.rbegin(), symbols_.rend(), [str](auto& p) { return p.first == str; }); it != symbols_.rend()) {
			return it->second;
		}
		throw std::runtime_error("unknown identifier \"" + std::string(str) + "\"");
	}

	void ident_put(const Identifier& ident);

	ParserContext();
};

struct non_term_t {};
constexpr non_term_t non_term{};

struct AstNode {
	struct number_t {
		bool as_floating_point;
		union {
			int value_int;
			float value_float;
		};

		template<typename BinOperator>
		static number_t perform_binary_op(const number_t& lhs, const number_t& rhs, BinOperator&& op) {
			number_t res;

			res.as_floating_point = lhs.as_floating_point || rhs.as_floating_point;

			if (res.as_floating_point) {
				float a, b;
				if (lhs.as_floating_point) a = lhs.value_float;
				else a = static_cast<float>(lhs.value_int);
				if (rhs.as_floating_point) b = rhs.value_float;
				else b = static_cast<float>(rhs.value_int);
				res.value_float = op(a, b);
			} else {
				res.value_int = op(lhs.value_int, rhs.value_int);
			}

			return res;
		}

		template<typename UnOperator>
		static number_t perform_unary_op(const number_t& num, UnOperator&& op) {
			number_t res;
			res.as_floating_point = num.as_floating_point;
			if (res.as_floating_point) res.value_float = op(num.value_float);
			else res.value_int = op(num.value_int);
			return res;
		}

		number_t operator-() {
			return perform_unary_op(*this, std::negate{});
		}

		number_t& operator+=(int value) {
			if (as_floating_point) {
				value_float += static_cast<float>(value);
			} else {
				value_int += value;
			}
			return *this;
		}

		number_t& operator+=(const number_t& other) {
			*this = perform_binary_op(*this, other, std::plus{});
			return *this;
		}

		number_t() = default;

		number_t(int x) 
			: as_floating_point{false}
			, value_int{x} {}
		
		number_t(float x) 
			: as_floating_point{true}
			, value_float{x} {}
	};

	using children_t = std::vector<AstNode*>;
	
	AstType type;
	std::variant<std::string, number_t, Identifier0, children_t> value;
	
	union {
		llvm::Value* llvm_value = nullptr;
	};

	std::string_view node_type_str() const noexcept { 
		auto idx = static_cast<size_t>(type);
		assert (idx < ast_nodes_size); 
		return a_node_type_str[idx]; 
	}
	
	bool is_non_teminal() const noexcept { return std::holds_alternative<children_t>(value); }

	template<typename... Rest>
	void push(Rest... rest) {
		assert(std::holds_alternative<children_t>(value));
		auto& children = std::get<children_t>(value);
		(std::invoke([&children](auto x) { if (x) children.push_back(x); }, rest), ...);
	}

	AstNode& operator=(const number_t& number) {
		type = AstType::Number;
		value = number;
		return *this;
	}

	number_t& as_number() { 
		assert(type == AstType::Number);
		return std::get<number_t>(value); 
	}

	const number_t& as_number() const { 
		assert(type == AstType::Number);
		return std::get<number_t>(value); 
	}

	Identifier0& as_ident() { 
		assert(type == AstType::Identifier);
		return std::get<Identifier0>(value); 
	}

	const Identifier0& as_ident() const { 
		assert(type == AstType::Identifier);
		return std::get<Identifier0>(value); 
	}

	auto begin() { return std::get<children_t>(value).begin(); }
	auto end() { return std::get<children_t>(value).end(); }

	void delete_children() {
		for (auto c : *this) { delete c; }
		std::get<children_t>(value).clear();
	}

	AstNode& operator[](size_t ix) {
		assert(std::holds_alternative<children_t>(value));
		return *(std::get<children_t>(value))[ix];
	}

	const AstNode& operator[](size_t ix) const {
		assert(std::holds_alternative<children_t>(value));
		return *(std::get<children_t>(value))[ix];
	}

	AstNode*& child_address_of(size_t ix) {
		assert(std::holds_alternative<children_t>(value));
		return std::get<children_t>(value)[ix];
	}

	AstNode& set_child(size_t ix, AstNode* n) {
		assert(std::holds_alternative<children_t>(value));
		return *(std::get<children_t>(value)[ix] = n);
	}


	static boost::pool<> pool_;

	static void* operator new(size_t) {
		return pool_.malloc();
	}

	static void operator delete(void* ptr) {
		pool_.free(ptr);
	}

	template<typename... Rest>
	AstNode(non_term_t, AstType _type, Rest&&... rest) 
		: type(_type)
		, value(std::vector<AstNode*>{})
	{ 
		push(rest...);
	}

	AstNode(AstType _type, const char* value_str) 
		: type(_type)
		, value(std::string(value_str)) {}

	AstNode(const char* value_str) 
		: type(AstType::StringLiteral)
		, value(std::string(value_str)) {}

	AstNode(int value_int) 
		: type(AstType::Number)
		, value(number_t{value_int}) {}
	
	AstNode(float value_float) 
		: type(AstType::Number)
		, value(number_t{value_float}) {}
	
	AstNode(Identifier0&& ident)
		: type(AstType::Identifier)
		, value(std::move(ident)) {}

	AstNode(const Identifier& ident)
		: type(AstType::Identifier)
		, value(std::move(Identifier0{ident})) {}

	AstNode(const AstNode&) = default;
	AstNode(AstNode&&) = default;

	AstNode& operator =(const AstNode&) = default;
	AstNode& operator =(AstNode&&) = default;
};

inline std::ostream& operator<<(std::ostream& os, const AstNode::number_t& num) {
	if (num.as_floating_point) os << num.value_float;
	else os << num.value_int;
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const Identifier0& ident) {
	if (ident.negate) os << '-';
	os << ident.name;
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const AstNode& n) {
	
	switch (n.type) {
	case AstType::Add:
		os << '+';
		break;
	case AstType::Mul:
		os << '*';
		break;
	case AstType::Number:
		os << n.as_number();
		break;
	case AstType::Identifier:
		os << n.as_ident();
		break;
	default:
		os << n.node_type_str();
	}
	return os;
}


inline boost::pool<> AstNode::pool_{sizeof(AstNode), 256};

inline AstNode::number_t operator+(const AstNode::number_t& lhs, const AstNode::number_t& rhs) {
	return AstNode::number_t::perform_binary_op(lhs, rhs, std::plus{});
}

inline AstNode::number_t operator*(const AstNode::number_t& lhs, const AstNode::number_t& rhs) {
	return AstNode::number_t::perform_binary_op(lhs, rhs, std::multiplies{});
}

template<typename... Func>
void traverse_postorder(AstNode* n, Func&&... fns) {
	if (n->is_non_teminal()) {
		auto& children = std::get<AstNode::children_t>(n->value);
		for (auto& c : children) {
			traverse_postorder(c, fns...);
		}
	}
	(fns(n), ...);
}

namespace detail {
inline void tree_height_r(const AstNode* n, size_t height, size_t& max_height) {
	if (max_height < height) max_height = height;
	if (n->is_non_teminal()) {
		auto& children = std::get<AstNode::children_t>(n->value);
		for (auto& c : children) tree_height_r(c, ++height, max_height);
	}
}
}

inline size_t tree_height(const AstNode& n) {
	size_t height = 0;
	detail::tree_height_r(&n, height, height);
	return height;
}

template<typename... Func>
generator<AstNode*> filter_branch_discard_dfs_preorder(AstNode& n, Func&&... fns) {
	assert(n.is_non_teminal());

	std::stack<std::pair<AstNode::children_t::iterator, AstNode::children_t::iterator>> stack;
	stack.push({
		std::get<AstNode::children_t>(n.value).begin(),
		std::get<AstNode::children_t>(n.value).end()
		});

	do {
		auto& cur = stack.top();
		if (cur.first != cur.second) {
			auto cur_ptr = *cur.first++;
			if ((... || fns(*(cur_ptr), stack.size() - 1))) {
				co_yield cur_ptr; // return top tree and discard sub trees
			} else {
				if (cur_ptr->is_non_teminal()) {
					stack.push({
						std::get<AstNode::children_t>(cur_ptr->value).begin(),
						std::get<AstNode::children_t>(cur_ptr->value).end()
						});
				}
			}
		} else {
			stack.pop();
		}
	} while (!stack.empty());
}

namespace impl {
template<bool use_filter, typename... Func>
generator<AstNode*> filter_dfs_preorder(AstNode& n, Func&&... fns) {
	assert(n.is_non_teminal());

	std::stack<std::pair<AstNode::children_t::iterator, AstNode::children_t::iterator>> stack;
	stack.push({
		std::get<AstNode::children_t>(n.value).begin(),
		std::get<AstNode::children_t>(n.value).end()
		});

	do {
		auto& cur = stack.top();
		if (cur.first != cur.second) {
			auto cur_ptr = *cur.first++;
			if constexpr (use_filter) {
				if ((... || fns(*(cur_ptr), stack.size(), cur.first == cur.second))) {
					co_yield cur_ptr;
				}
			} else {
				co_yield cur_ptr;
			}
			if (cur_ptr->is_non_teminal()) {
				stack.push({
					std::get<AstNode::children_t>(cur_ptr->value).begin(),
					std::get<AstNode::children_t>(cur_ptr->value).end()
					});
			}
		} else {
			stack.pop();
		}
	} while (!stack.empty());
}
} // namespace impl

inline generator<AstNode*> dfs_preorder(AstNode& n) {
	return impl::filter_dfs_preorder<false>(n);
}

template<typename... Func>
generator<AstNode*> filter_dfs_preorder(AstNode& n, Func&&... fns) {
	return impl::filter_dfs_preorder<true>(n, std::forward<Func>(fns)...);
}


template<typename... Func>
generator<std::pair<AstNode*, size_t>> bfs_preorder(AstNode& n) {
	assert(n.is_non_teminal());

	std::queue<std::pair<AstNode*, size_t>> que;

	que.emplace(&n, 0);

	do {
		auto& cur = que.front();
		if (cur.first->is_non_teminal()) {
			const size_t next_level = cur.second + 1;
			for (auto c : std::get<AstNode::children_t>(cur.first->value)) {
				que.emplace(c, next_level);
			}
		}
		co_yield cur;
		que.pop();
	} while (!que.empty());


}


inline void ParserContext::ident_put(const Identifier& ident) {
	auto rbegin = symbols_.rend() - scope_.back();
	auto it = std::find_if(rbegin, symbols_.rend(), [&ident](auto& p) { return p.first == ident.name; });
	if (it != symbols_.rend()) throw std::runtime_error("duplicated indentifier \"" + std::string(ident.name) + "\"");
	symbols_.emplace_back(ident.name, ident);
	++scope_.back();
}

inline ParserContext::ParserContext() {
	push_scope();
}

inline bool is_number(const AstNode& n) {
	return n.type == AstType::Number;
}

//void do_constant_folding(ParserContext& ctx, AstNode& root);

} // namespace npcompiler::ast

#endif // AST_HPP__
