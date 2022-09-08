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

#include <nplib/utils/types.h>
#include <boost/pool/object_pool.hpp>
#include <boost/container/small_vector.hpp>
#include <npcompiler/resolver.hpp>
#include "number.hpp"

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
	AST_TYPE(Number), /* expressions begin */ \
	AST_TYPE(Identifier), \
	AST_TYPE(Parameter), \
	AST_TYPE(Cast), \
	AST_TYPE(Add), \
	AST_TYPE(Mul), \
	AST_TYPE(Div), \
	AST_TYPE(Uminus) /* expressions end */

namespace llvm {
class Value;
}

namespace npcompiler::ast {

struct AstNode;

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

struct CastTo {
	int type;
};

struct Identifier {
	std::string name;
	int type;
	::llvm::Value* llvm_value;
};

struct Parameter {
	std::string path;
	npsys::variable_n var;
	//::llvm::Value* ptr;
};

struct Number {
	//::llvm::Value* llvm_value = nullptr;
	std::variant<fl::Integer, float> value;
	
	bool is_float() const noexcept {
		return std::holds_alternative<float>(value);
	}

//	template<typename BinOperator>
//	static Number perform_binary_op(const Number& lhs, const Number& rhs, BinOperator&& op) {
//		if (lhs.is_float() || rhs.is_float()) {
//			float a = lhs.is_float() ? std::get<float>(lhs.value) : static_cast<float>(std::get<int>(lhs.value));
//			float b = rhs.is_float() ? std::get<float>(rhs.value) : static_cast<float>(std::get<int>(rhs.value));
//			return Number(op(a, b));
//		} else {
//			return Number(op(std::get<int>(lhs.value), std::get<int>(rhs.value)));
//		}
//	}
//
//	template<typename UnOperator>
//	static Number perform_unary_op(const Number& num, UnOperator&& op) {
//		return Number(num.is_float() ? op(std::get<float>(num.value)) : op(std::get<int>(num.value)));
//	}
//
//	Number operator-() {
//		return perform_unary_op(*this, std::negate{});
//	}

	operator std::int32_t () {
		assert(!is_float());
		return std::get<fl::Integer>(value).x;
	}

	operator float() {
		assert(is_float());
		return std::get<float>(value);
	}

	const fl::Integer& as_int() const noexcept {
		return std::get<fl::Integer>(value);
	}

	fl::FDType get_type() const noexcept {
		return !is_float() ? as_int().type : npsys::nptype::NPT_F32;
	}

	explicit Number() = default;
	explicit Number(fl::Integer&& x) : value{x} {}
	explicit Number(float x) : value{x} {}
};

namespace detail {
template<typename T> struct cast {};
template<> struct cast<Number> { static constexpr AstType value = AstType::Number; };
template<> struct cast<Identifier> { static constexpr AstType value = AstType::Identifier; };
template<> struct cast<Parameter> { static constexpr AstType value = AstType::Parameter; };
template<> struct cast<CastTo> { static constexpr AstType value = AstType::Cast; };
}

using symbols_t = std::map<std::string, AstNode*, std::less<>>;

class ParserContext {
	globals_resolver_cb_t& globals_resolver_cb_;
public:
	bool error = false;
	bool global;
	symbols_t symbols_global;
	symbols_t symbols_local;

	[[nodiscard]] 
	AstNode* ident_create(std::string_view name, int type);
	AstNode* ident_get(std::string_view str);
	AstNode* ext_ident_get(std::string_view str) noexcept;

	AstNode* create_binary_op(AstType type, AstNode* lhs, AstNode* rhs);
	AstNode* create_assignment(AstNode* lhs, AstNode* rhs);


	ParserContext(globals_resolver_cb_t& cb)
		: globals_resolver_cb_{ cb } {}
};

struct non_term_t {};
constexpr non_term_t non_term{};

struct AstNode {
	using pool_t = boost::object_pool<AstNode>;
	using children_t = boost::container::small_vector<AstNode*, 2>;

	AstType type;
	children_t children;
	std::variant<Number, Identifier, Parameter, CastTo> data;

	auto begin() noexcept { return children.begin(); }
	auto end() noexcept { return children.end(); }

	auto begin() const noexcept { return children.cbegin(); }
	auto end() const noexcept { return children.cend(); }

	std::string_view node_type_str() const noexcept {
		auto idx = static_cast<size_t>(type);
		assert(idx < ast_nodes_size);
		return a_node_type_str[idx];
	}

	bool is_expression() const noexcept {
		return type >= AstType::Number;
	}

	template<typename... Rest>
	void push(Rest... rest) {
		(std::invoke([this](auto x) { if (x) children.push_back(x); }, rest), ...);
	}

	template<typename T>
	T& cast() noexcept {
		assert(type == detail::cast<T>::value);
		return std::get<T>(data);
	}

	template<typename T>
	const T& cast() const noexcept {
		assert(type == detail::cast<T>::value);
		return std::get<T>(data);
	}

	AstNode& operator[](size_t ix) {
		return *children[ix];
	}

	const AstNode& operator[](size_t ix) const {
		return *children[ix];
	}

	AstNode*& child_address_of(size_t ix) {
		return children[ix];
	}

	AstNode& set_child(size_t ix, AstNode* n) {
		return *(children[ix] = n);
	}

	int get_expression_type() {
		assert(is_expression());
		int type = -1;

		std::visit([&type] <typename T> (const T & t) { 
			if constexpr (std::is_same_v<T, Number>) {
				type = t.get_type();
			} else if constexpr (std::is_same_v<T, Identifier>) {
				type = t.type;
			} else if constexpr (std::is_same_v<T, Parameter>) {
				type = t.var->GetClearType();
			} else if constexpr (std::is_same_v<T, CastTo>) {
				type = t.type;
			}
		}, data);

		assert(type != -1);

		return type;
	}

	inline static pool_t* pool_;

	static void* operator new(size_t) {
		return pool_->malloc();
	}

	static void operator delete(void* ptr) {
		assert(false);
	}

	template<typename... Rest>
	AstNode(AstType _type, Rest&&... rest)
		: type(_type)
	{
		push(rest...);
	}

	template<typename T, typename... Rest>
	AstNode(std::tuple<AstType&&, T&&> type_val, Rest&&... rest)
		: type(get<0>(type_val))
		, data(get<1>(type_val))
	{
		push(rest...);
	}
	
	AstNode(fl::Integer&& i)
		: type(AstType::Number)
		, data{ Number (std::move(i)) } {}

	AstNode(float value_float)
		: type(AstType::Number)
		, data{ Number (value_float) } {}

	AstNode(Identifier&& ident)
		: type(AstType::Identifier)
		, data(std::move(ident)) {}

	AstNode(Parameter&& param)
		: type(AstType::Parameter)
		, data(std::move(param)) {}


	AstNode(const AstNode&) = default;
	AstNode(AstNode&&) = default;

	AstNode& operator = (const AstNode&) = default;
	AstNode& operator = (AstNode&&) = default;
};

inline std::ostream& operator<<(std::ostream& os, const Number& num) {
	std::visit([&os] <typename T> (const T & t) { os << t; }, num.value);
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
		os << n.cast<Number>();
		break;
	case AstType::Identifier:
		os << n.cast<Identifier>();
		break;
	default:
		os << n.node_type_str();
		break;
	}
	return os;
}

// inline Number operator+(const Number& lhs, const Number& rhs) {
// 	return Number::perform_binary_op(lhs, rhs, std::plus{});
// }
// 
// inline Number operator*(const Number& lhs, const Number& rhs) {
// 	return Number::perform_binary_op(lhs, rhs, std::multiplies{});
// }

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
}

inline AstNode* ParserContext::ext_ident_get(std::string_view str) noexcept {
	try {
		return new AstNode(Parameter{ std::string(str), globals_resolver_cb_(str) });
	} catch (std::exception& ex) {
		std::cerr << ex.what() << '\n';
	}
	return new AstNode(Parameter{ std::string(str) });
}

inline bool is_number(const AstNode& n) {
	return n.type == AstType::Number;
}

} // namespace npcompiler::ast

#endif // AST_HPP__
