// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <string>
#include <tuple>
#include <vector>
#include <optional>
#include <cassert>
#include <filesystem>
#include <variant>
#include <algorithm>

constexpr int fundamental_type_first = 256;
constexpr int fundamental_type_last = fundamental_type_first + 16;

#define ONE_CHAR_TOKENS() \
	TOKEN_FUNC(Hash, '#') \
	TOKEN_FUNC(RoundBracketOpen, '(') \
	TOKEN_FUNC(RoundBracketClose, ')') \
	TOKEN_FUNC(Comma, ',') \
	TOKEN_FUNC(Semicolon, ';') \
	TOKEN_FUNC(Assignment, '=') \
	TOKEN_FUNC(Optional, '?') \
	TOKEN_FUNC(Less, '<') \
	TOKEN_FUNC(Greater, '>') \
	TOKEN_FUNC(SquareBracketOpen, '[') \
	TOKEN_FUNC(SquareBracketClose, ']') \
	TOKEN_FUNC(BracketOpen, '{') \
	TOKEN_FUNC(BracketClose, '}')

enum class TokenId {
	Colon = ':',

#define TOKEN_FUNC(x, y) x = y,
	ONE_CHAR_TOKENS()
#undef TOKEN_FUNC

	Boolean = fundamental_type_first,
	Int8,
	UInt8,
	Int16,
	UInt16,
	Int32,
	UInt32,
	Int64,
	UInt64,
	Float32,
	Float64 = fundamental_type_last,

	Identifier,
	Number,
	String,
	Vector,
	Flat,
	Exception,
	Eof,
	DoubleColon,
	Namespace,
	Interface,
	Object,
	Void,
	In,
	Out,
	Enum,
	Using,
	Raises,
	OutDirect,
	Helper,
	Trusted,
	Async,
	Const,
};

struct AstStructDecl;
struct AstTypeDecl;
struct AstInterfaceDecl;
struct AstFunctionDecl;
struct AstNumber;

class Namespace {
	Namespace* parent_;
	std::string name_;

	std::vector<Namespace*> children_;
	std::vector<std::pair<std::string, AstTypeDecl*>> types_;
	std::vector<std::pair<std::string, AstNumber>> constants_;

	std::string construct_path(std::string delim, bool omit_root = false) const noexcept;
public:
	Namespace();
	Namespace(Namespace* parent, std::string&& name);

	auto push(std::string&& s) noexcept;
	AstTypeDecl* find_type(const std::string& str, bool only_this_namespace);
	Namespace* find_child(const std::string& str);
	AstNumber* find_constant(const std::string& name);
	void add(const std::string& name, AstTypeDecl* type);
	void add_constant(std::string&& name, AstNumber&& number);
	
	const std::string& name() const noexcept;
	Namespace* parent() const noexcept;
	std::string to_cpp17_namespace() const noexcept;
	std::string to_ts_namespace(bool omit_root = true) const noexcept;
};

enum class FieldType {
	Fundamental,
	Array,
	Vector,
	String,
	Struct,
	Optional,
	Void,
	Object,
	Interface,
	Alias,
	Enum,
};

enum class NumberFormat { Decimal, Hex, Scientific };

struct AstNumber {
	std::variant<std::int64_t, float, double, bool> value;
	NumberFormat format;
	int max_size = 0;

	std::int64_t decimal() const noexcept {
		assert(std::holds_alternative<std::int64_t>(value));
		return std::get<std::int64_t>(value);
	}

	bool is_decimal() const noexcept {
		return std::holds_alternative<std::int64_t>(value);
	}
};

inline bool operator == (std::int64_t x, const AstNumber& n) {
	assert(std::holds_alternative<std::int64_t>(n.value));
	return std::get<std::int64_t>(n.value) == x;
}

inline bool operator != (std::int64_t x, const AstNumber& n) {
	return !::operator==(x, n);
}

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
//template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

inline std::ostream& operator << (std::ostream& os, const AstNumber& n) {
	std::visit(overloaded{
		[&](int64_t x) { 
			if (n.format == NumberFormat::Hex) {
				os << "0x" << std::hex;
				if (n.max_size) {
					os << std::setfill('0') << std::setw(static_cast<std::streamsize>(n.max_size) * 2);
				}
				os << x;
				os << std::dec;
			} else {
				os << x;
			}
		},
		[&](float x) { os << x; },
		[&](double x) { os << x; },
		[&](bool x) { os << x; },
		}, n.value);
	return os;
}


struct AstTypeDecl {
	FieldType id;
};

struct AstObjectDecl : AstTypeDecl {
	AstObjectDecl() {
		id = FieldType::Object;
	}
};

struct AstVoidDecl : AstTypeDecl {
	AstVoidDecl() {
		id = FieldType::Void;
	}
};

struct AstFundamentalType : AstTypeDecl {
	TokenId token_id;

	AstFundamentalType(TokenId _token_id)
		: token_id(_token_id) 
	{
		id = FieldType::Fundamental;
	}
};

struct AstWrapType : AstTypeDecl {
	AstTypeDecl* type;
	AstWrapType(AstTypeDecl* _type) : type(_type) {}

	AstTypeDecl* real_type();
};

struct AstArrayDecl : AstWrapType {
	const int length;

	AstArrayDecl(AstTypeDecl* _type, int _length) 
		: AstWrapType(_type)
		, length(_length) 
	{
		id = FieldType::Array;
	}
};

struct AstVectorDecl : AstWrapType {
	AstVectorDecl()
		: AstWrapType(nullptr) 
	{
		id = FieldType::Vector;
	}
};

struct AstAliasDecl : AstWrapType {
	Namespace* nm;
	std::string name;

	AstTypeDecl* get_real_type() const noexcept {
		auto t = this;
		while (t->type->id == FieldType::Alias)
			t = static_cast<AstAliasDecl*>(t->type);
		return t->type;
	}

	AstAliasDecl(std::string&& _name,Namespace* _nm, AstTypeDecl* _type)
		: AstWrapType(_type)
		, nm(_nm)
		, name(_name)
	{
		id = FieldType::Alias;
	}
};

struct AstStringDecl : AstTypeDecl {
	AstStringDecl() {
		id = FieldType::String;
	}
};

struct AstInterfaceDecl : AstTypeDecl {
	std::string name;
	std::vector<AstFunctionDecl*> fns;
	std::vector<AstInterfaceDecl*> plist;
	bool trusted = true;


	AstInterfaceDecl() {
		id = FieldType::Interface;
	}
};

struct AstEnumDecl : AstFundamentalType {
	std::string name;
	Namespace* nm;
	std::vector<std::pair<std::string, std::pair<AstNumber, bool>>> items;
	
	AstEnumDecl() 
		: AstFundamentalType(TokenId::UInt32) 
	{
		id = FieldType::Enum;
	}
};

struct AstFieldDecl {
	AstTypeDecl* type;
	std::string name;
	bool function_argument = false;
	bool input_function_argument;
	std::string_view function_name;
	std::string_view function_argument_name;
	
	bool is_optional() const noexcept {
		return type->id == FieldType::Optional;
	}
};

using struct_id_t = std::string;

struct AstStructDecl : AstTypeDecl {
	int version = -1;
	Namespace* nm;
	std::string name;
	struct_id_t unique_id;
	std::vector<AstFieldDecl*> fields;
	int size = -1;
	int align = -1;
	bool flat = true;
	bool has_span_class = false;
	int exception_id;

	bool is_exception() const noexcept { return exception_id != -1; }
	const struct_id_t& get_function_struct_id();

	AstStructDecl() {
		id = FieldType::Struct;
	}
};

struct AstOptionalDecl : AstWrapType {
	AstOptionalDecl(AstTypeDecl* _type)
		: AstWrapType(_type)
	{
		id = FieldType::Optional;
	}
};

constexpr auto cft(AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Fundamental || type->id == FieldType::Enum);
	return static_cast<AstFundamentalType*>(type);
}

constexpr auto cft(const AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Fundamental || type->id == FieldType::Enum);
	return static_cast<const AstFundamentalType*>(type);
}

constexpr auto cwt(AstTypeDecl* type) noexcept {
	assert(
		type->id == FieldType::Array ||
		type->id == FieldType::Vector ||
		type->id == FieldType::Alias ||
		type->id == FieldType::Optional
	);
	return static_cast<AstWrapType*>(type);
}

constexpr auto car(AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Array);
	return static_cast<AstArrayDecl*>(type);
}

constexpr auto cvec(AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Vector);
	return static_cast<AstVectorDecl*>(type);
}

constexpr auto cstr(AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::String);
	return static_cast<AstStringDecl*>(type);
}

constexpr auto cobj(AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Object);
	return static_cast<AstObjectDecl*>(type);
}

constexpr auto cflat(AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Struct);
	return static_cast<AstStructDecl*>(type);
}

constexpr auto cenum(AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Enum);
	return static_cast<AstEnumDecl*>(type);
}

constexpr auto cenum(const AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Enum);
	return static_cast<const AstEnumDecl*>(type);
}

constexpr auto calias(AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Alias);
	return static_cast<AstAliasDecl*>(type);
}

constexpr auto calias(const AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Alias);
	return static_cast<const AstAliasDecl*>(type);
}

constexpr auto cifs(AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Interface);
	return static_cast<AstInterfaceDecl*>(type);
}

constexpr auto copt(AstTypeDecl* type) noexcept {
	assert(type->id == FieldType::Optional);
	return static_cast<AstOptionalDecl*>(type);
}


inline AstTypeDecl* AstWrapType::real_type() {
	auto wt = type;
	if (wt->id == FieldType::Alias) wt = calias(wt)->get_real_type();
	return wt;
}

enum class ArgumentModifier { In, Out };

struct AstFunctionArgument : AstFieldDecl {
	ArgumentModifier modifier;
	bool direct = false;
};

struct AstFunctionDecl {
	uint16_t idx;
	std::string name;
	AstTypeDecl* ret_value;
	AstStructDecl* in_s = nullptr;
	AstStructDecl* out_s = nullptr;
	bool arguments_structs_have_been_made = false;
	AstStructDecl* ex = nullptr;
	std::vector<AstFunctionArgument*> args, in_args, out_args;
	bool is_async;

	bool is_void() const noexcept { return ret_value->id == FieldType::Void; }
};


template<typename IdType, typename T>
class List {
	static_assert(std::is_pointer_v<T>);
	std::vector<std::tuple<IdType, T>> items_;
public:
	T get(const IdType& id) {
		if (auto it = std::find_if(items_.begin(), items_.end(),
			[id](auto& t) { return std::get<0>(t) == id; }); it != items_.end()) {
			return std::get<1>(*it);
		}
		return nullptr;
	}
	void put(const IdType& id, T item) {
		items_.emplace_back(id, item);
	}
	auto begin() { return items_.begin(); }
	auto end() { return items_.end(); }
};


using AFFAList = List<struct_id_t, AstStructDecl*>;

inline const std::string& Namespace::name() const noexcept { return name_; }
inline Namespace* Namespace::parent() const noexcept { return parent_; }

inline auto Namespace::push(std::string&& s) noexcept {
	children_.push_back(new Namespace(this, std::move(s)));
	return children_.back();
}

inline AstTypeDecl* Namespace::find_type(const std::string& str, bool only_this_namespace) {
	if (auto it = std::find_if(types_.begin(), types_.end(),
		[&str](auto const& pair) { return pair.first == str; }); it != types_.end()) {
		return it->second;
	}

	if (only_this_namespace == false && parent()) return parent()->find_type(str, false);

	return nullptr; // throw_error("Unknown type: \"" + str + "\"");
}

inline Namespace* Namespace::find_child(const std::string& str) {
	if (auto it = std::find_if(children_.begin(), children_.end(),
		[&str](auto nm) {return nm->name() == str; }); it != children_.end()) {
		return *it;
	}
	return nullptr;
}

inline void Namespace::add(const std::string& name, AstTypeDecl* type) {
	if (std::find_if(std::begin(types_), std::end(types_), [&name](const auto& pair) { return pair.first == name; }) != std::end(types_)) {
		throw "type redefinition";
	}
	types_.push_back({ name, type });
}

inline std::string Namespace::construct_path(std::string delim, bool omit_root) const noexcept {
	if (omit_root && parent() && parent()->name().empty()) return {};
	
	auto ptr = parent();
	std::string result = name();
	while (ptr && !ptr->name().empty()) {
		if (omit_root && ptr->parent() && ptr->parent()->name().empty()) break;
		result = ptr->name() + delim + result;
		ptr = ptr->parent();
	}
	return result;
}

inline std::string Namespace::to_cpp17_namespace() const noexcept {
	return construct_path(std::string(2, ':'));
}

inline std::string Namespace::to_ts_namespace(bool omit_root) const noexcept {
	return construct_path(std::string(1, '.'), omit_root);
}

inline Namespace::Namespace()
	: parent_(nullptr)
{
}

inline Namespace::Namespace(Namespace* parent, std::string&& name)
	: parent_(parent)
	, name_(std::move(name))
{
}

inline void Namespace::add_constant(std::string&& name, AstNumber&& number) {
	if (std::find_if(std::begin(constants_), std::end(constants_), 
		[&name](const auto& pair) { return pair.first == name; }) != std::end(constants_)) {
		throw std::runtime_error("constant redefinition");
	}
	constants_.emplace_back(std::move(name), std::move(number));
}

inline AstNumber* Namespace::find_constant(const std::string& name) {
	auto it = std::find_if(std::begin(constants_), std::end(constants_),
		[&name](const auto& pair) { return pair.first == name; });
	return it != std::end(constants_) ? &it->second : nullptr;
}

class Context {
	Namespace* nm_root_;
	Namespace* nm_cur_;
	int exception_id_last = -1;

	std::string module_name;
	std::string base_name;
public:
	AFFAList affa_list;
	int m_struct_n_ = 0;
	std::vector<AstStructDecl*> exceptions;
	std::vector<AstInterfaceDecl*> interfaces;
	std::vector<AstStructDecl*> structs_with_helpers;

	const std::string& current_file() const noexcept {
		return base_name;
	}

	const std::string& module() const noexcept {
		return module_name;
	}

	std::string top_level_namespace() const noexcept {
		return module_name + "::" + base_name;
	}

	int next_exception_id () noexcept {
		return ++exception_id_last; 
	}

	auto push_namespace(std::string&& s) {
		nm_cur_ = nm_cur_->push(std::move(s));
		return nm_cur_;
	}

	void pop_namespace() {
		nm_cur_ = nm_cur_->parent();
		assert(nm_cur_);
	}

	Namespace* nm_cur() {
		return nm_cur_;
	}
	
	Namespace* nm_root() {
		return nm_root_; 
	}

	Namespace* set_namespace(Namespace* nm) {
		auto old = nm_cur_;
		nm_cur_ = nm;
		return old;
	}

	bool is_nprpc_base() const noexcept {
		return base_name == "nprpc_base";
	}

	bool is_nprpc_nameserver() const noexcept {
		return base_name == "nprpc_nameserver";
	}

	void open(std::filesystem::path file_path) {
		base_name = file_path.filename().replace_extension().string();
		std::transform(base_name.begin(), base_name.end(), base_name.begin(), [](char c) {
			return c == '.' ? '_' : ::tolower(c); });
		nm_root_ = nm_cur_ = new Namespace();
	}

	Context(const std::string& module_name)
		: module_name(module_name)
	{
	}
};
