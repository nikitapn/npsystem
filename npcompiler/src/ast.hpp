// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#ifndef AST_HPP__
#define AST_HPP__

#include <iostream>
#include <vector>
#include <stack>
#include <array>
#include <map>
#include <string>
#include <string_view>
#include <cassert>
#include <variant>
#include <queue>
#include <memory>
#include <variant>
#include <functional>
#include <optional>
#include <coroutine>
#include <experimental/generator>

#include <boost/pool/object_pool.hpp>
#include <boost/container/small_vector.hpp>


using std::experimental::generator;

template<class... Ts> struct overloaded : Ts... {};
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#define AST_NODE_TYPES() \
	AST_TYPE(Module), \
	AST_TYPE(Program), \
	AST_TYPE(Program_End), \
	AST_TYPE(Function), \
	AST_TYPE(Function_End), \
	AST_TYPE(FunctionBlock), \
	AST_TYPE(StmtList), \
	AST_TYPE(StringLiteral), \
	AST_TYPE(LocalVarDeclSeq), \
	AST_TYPE(GlobalVarDeclSeq), \
	AST_TYPE(Assignment), \
	AST_TYPE(If), \
	AST_TYPE(Number), \
	AST_TYPE(Identifier), \
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

struct Identifier {
	std::string name;
	int type;
	llvm::Value* ptr;
};

struct Number {
	bool as_floating_point;

	union {
		int value_int;
		float value_float;
	};

	template<typename BinOperator>
	static Number perform_binary_op(const Number& lhs, const Number& rhs, BinOperator&& op) {
		Number res;

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
	static Number perform_unary_op(const Number& num, UnOperator&& op) {
		Number res;
		res.as_floating_point = num.as_floating_point;
		if (res.as_floating_point) res.value_float = op(num.value_float);
		else res.value_int = op(num.value_int);
		return res;
	}

	Number operator-() {
		return perform_unary_op(*this, std::negate{});
	}

	Number& operator+=(int value) {
		if (as_floating_point) {
			value_float += static_cast<float>(value);
		} else {
			value_int += value;
		}
		return *this;
	}

	Number& operator+=(const Number& other) {
		*this = perform_binary_op(*this, other, std::plus{});
		return *this;
	}

	Number() = default;

	Number(int x)
		: as_floating_point{ false }
		, value_int{ x } {}

	Number(float x)
		: as_floating_point{ true }
		, value_float{ x } {}
};

struct AstNode;

using symbols_t = std::map<std::string, AstNode*, std::less<>>;

struct ParserContext {
	bool error = false;
	bool global;
	symbols_t symbols_global;
	symbols_t symbols_local;

	[[nodiscard]]
	AstNode* ident_create(std::string_view name, int type);
	AstNode* ident_get(std::string_view str);
	AstNode* ext_ident_get(std::string_view str);

};

struct non_term_t {};
constexpr non_term_t non_term{};

struct AstNode {
	using pool_t = boost::object_pool<AstNode>;
	using children_t = boost::container::small_vector<AstNode*, 2>;
	
	AstType type;
	std::variant<std::string, Number, Identifier, children_t> value;
	
	union {
		llvm::Value* llvm_value = nullptr;
	};

	std::string_view node_type_str() const noexcept { 
		auto idx = static_cast<size_t>(type);
		assert (idx < ast_nodes_size); 
		return a_node_type_str[idx]; 
	}

	bool is_expression() const noexcept {
		return type >= AstType::Number;
	}

	bool is_non_teminal() const noexcept { return std::holds_alternative<children_t>(value); }

	template<typename... Rest>
	void push(Rest... rest) {
		assert(std::holds_alternative<children_t>(value));
		auto& children = std::get<children_t>(value);
		(std::invoke([&children](auto x) { if (x) children.push_back(x); }, rest), ...);
	}

	AstNode& operator=(const Number& number) {
		type = AstType::Number;
		value = number;
		return *this;
	}

	Number& as_number() { 
		assert(type == AstType::Number);
		return std::get<Number>(value); 
	}

	const Number& as_number() const { 
		assert(type == AstType::Number);
		return std::get<Number>(value); 
	}

	Identifier& as_ident() { 
		assert(type == AstType::Identifier);
		return std::get<Identifier>(value); 
	}

	const Identifier& as_ident() const { 
		assert(type == AstType::Identifier);
		return std::get<Identifier>(value); 
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


	inline static pool_t* pool_;

	static void* operator new(size_t) {
		return pool_->malloc();
	}

	static void operator delete(void* ptr) {
		assert(false);
	}

	template<typename... Rest>
	AstNode(non_term_t, AstType _type, Rest&&... rest) 
		: type(_type)
		, value(children_t{})
	{ 
		push(rest...);
	}

	AstNode(AstType _type)
		: type(_type) {}
	
	AstNode(AstType _type, const char* value_str) 
		: type(_type)
		, value(std::string(value_str)) {}

	AstNode(const char* value_str) 
		: type(AstType::StringLiteral)
		, value(std::string(value_str)) {}

	AstNode(int value_int) 
		: type(AstType::Number)
		, value(Number{value_int}) {}
	
	AstNode(float value_float) 
		: type(AstType::Number)
		, value(Number{value_float}) {}
	
	AstNode(Identifier&& ident)
		: type(AstType::Identifier)
		, value(std::move(ident)) {}

	AstNode(const AstNode&) = default;
	AstNode(AstNode&&) = default;

	AstNode& operator =(const AstNode&) = default;
	AstNode& operator =(AstNode&&) = default;
};

inline std::ostream& operator<<(std::ostream& os, const Number& num) {
	if (num.as_floating_point) os << num.value_float;
	else os << num.value_int;
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const Identifier& ident) {
	//if (ident.negate) os << '-';
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


inline Number operator+(const Number& lhs, const Number& rhs) {
	return Number::perform_binary_op(lhs, rhs, std::plus{});
}

inline Number operator*(const Number& lhs, const Number& rhs) {
	return Number::perform_binary_op(lhs, rhs, std::multiplies{});
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


inline AstNode* ParserContext::ident_create(std::string_view name, int type) {
	auto n = std::make_unique<AstNode>(Identifier{ std::string(name), type });
	
	if (!global) [[likely]] {
		if (symbols_local.find(name) != symbols_local.end()) {
			throw std::runtime_error("duplicated identifier \"" + std::string(name) + "\"");
		}
		symbols_local.emplace(name, n.get());
	} else {
		if (symbols_global.find(name) != symbols_global.end()) {
			throw std::runtime_error("duplicated identifier \"" + std::string(name) + "\"");
		}
		symbols_global.emplace(name, n.get());
	}
	
	return n.release();
}

inline AstNode* ParserContext::ident_get(std::string_view str) {
	if (auto it = symbols_local.find(str); it != symbols_local.end()) {
		return it->second;
	}
	throw std::runtime_error("unknown identifier \"" + std::string(str) + "\"");
	return nullptr;
}

inline bool is_number(const AstNode& n) {
	return n.type == AstType::Number;
}

//void do_constant_folding(ParserContext& ctx, AstNode& root);

} // namespace npcompiler::ast

#endif // AST_HPP__
