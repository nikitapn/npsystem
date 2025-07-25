// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "ts_builder.hpp"
#include <cassert>
#include <string_view>
#include <algorithm>
#include "utils.hpp"
#include <map>

static std::string_view fundamental_to_ts(TokenId id);

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

static const int token_mod_addr = std::ios_base::xalloc();

template<int _Mod> 
struct token_os_mod {
	static constexpr int _mod = _Mod;
};

template<int _Write>
struct read_write_field : token_os_mod<_Write> {
	inline static int offset_addr = std::ios_base::xalloc();
	int offset;
	explicit read_write_field(int o) : offset(o) {}
};

template<int _Mod>
static std::ostream& operator << (std::ostream& os, const token_os_mod<_Mod>& /* field */) {
	os.iword(token_mod_addr) = token_os_mod<_Mod>::_mod;
	return os;
}

template<int _Mod>
static std::ostream& operator << (std::ostream& os, const read_write_field<_Mod>& field) {
	os.iword(token_mod_addr) = read_write_field<_Mod>::_mod;
	os.iword(read_write_field<_Mod>::offset_addr) = field.offset;
	return os;
}

using read_field = read_write_field<0>;
using write_field = read_write_field<1>;
using _token_type = token_os_mod<2>;
constexpr auto toktype = _token_type{};

static std::ostream& operator << (std::ostream& os, const TokenId& token_id) {
	const auto token_mod = os.iword(token_mod_addr);
	if (token_mod == 0) {
		const auto offset = os.iword(read_field::offset_addr);
		switch (token_id) {
		case TokenId::Boolean:
			os << "(this.buffer.dv.getUint8" << "(this.offset+" << offset << ") === 0x01)";
			break;
		case TokenId::Int8:
			os << "this.buffer.dv.getInt8" << "(this.offset+" << offset << ")";
			break;
		case TokenId::UInt8:
			os << "this.buffer.dv.getUint8" << "(this.offset+" << offset << ")";
			break;
		case TokenId::Int16:
			os << "this.buffer.dv.getInt16" << "(this.offset+" << offset << ",true)";
			break;
		case TokenId::UInt16:
			os << "this.buffer.dv.getUint16" << "(this.offset+" << offset << ",true)";
			break;
		case TokenId::Int32:
			os << "this.buffer.dv.getInt32" << "(this.offset+" << offset << ",true)";
			break;
		case TokenId::UInt32:
			os << "this.buffer.dv.getUint32" << "(this.offset+" << offset << ",true)";
			break;
		case TokenId::Int64:
			os << "this.buffer.dv.getBigInt64" << "(this.offset+" << offset << ",true)";
			break;
		case TokenId::UInt64:
			os << "this.buffer.dv.getBigUint64" << "(this.offset+" << offset << ",true)";
			break;
		case TokenId::Float32:
			os << "this.buffer.dv.getFloat32" << "(this.offset+" << offset << ",true)";
			break;
		case TokenId::Float64:
			os << "this.buffer.dv.getFloat64" << "(this.offset+" << offset << ",true)";
			break;
		default:
			assert(false);
		}
	} else if (token_mod == 1) {
		const auto offset = os.iword(read_field::offset_addr);
		switch (token_id) {
		case TokenId::Boolean:
			os << "this.buffer.dv.setUint8" << "(this.offset+" << offset << ", value === true ? 0x01 : 0x00)";
			break;
		case TokenId::Int8:
			os << "this.buffer.dv.setInt8" << "(this.offset+" << offset << ",value)";
			break;
		case TokenId::UInt8:
			os << "this.buffer.dv.setUint8" << "(this.offset+" << offset << ",value)";
			break;
		case TokenId::Int16:
			os << "this.buffer.dv.setInt16" << "(this.offset+" << offset << ",value,true)";
			break;
		case TokenId::UInt16:
			os << "this.buffer.dv.setUint16" << "(this.offset+" << offset << ",value,true)";
			break;
		case TokenId::Int32:
			os << "this.buffer.dv.setInt32" << "(this.offset+" << offset << ",value,true)";
			break;
		case TokenId::UInt32:
			os << "this.buffer.dv.setUint32" << "(this.offset+" << offset << ",value,true)";
			break;
		case TokenId::Int64:
			os << "this.buffer.dv.setBigInt64" << "(this.offset+" << offset << ",value,true)";
			break;
		case TokenId::UInt64:
			os << "this.buffer.dv.setBigUint64" << "(this.offset+" << offset << ",value,true)";
			break;
		case TokenId::Float32:
			os << "this.buffer.dv.setFloat32" << "(this.offset+" << offset << ",value,true)";
			break;
		case TokenId::Float64:
			os << "this.buffer.dv.setFloat64" << "(this.offset+" << offset << ",value,true)";
			break;
		default:
			assert(false);
		}
	} else if (token_mod == 2) {
		//os << fundamental_to_ts(token_id);
		
		switch (token_id) {
		case TokenId::Boolean:	os << "boolean"; break;
		case TokenId::Int8:			os << "i8"; break;
		case TokenId::UInt8:		os << "u8"; break;
		case TokenId::Int16:		os << "i16"; break;
		case TokenId::UInt16:		os << "u16"; break;
		case TokenId::Int32:		os << "i32"; break;
		case TokenId::UInt32:		os << "u32"; break;
		case TokenId::Int64:		os << "i64"; break;
		case TokenId::UInt64:		os << "u64"; break;
		case TokenId::Float32:	os << "f32"; break;
		case TokenId::Float64:	os << "f64"; break;
		default: assert(false);
		}

	} else {
		assert(false);
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const TypescriptBuilder::_ns& ns) {
	if (
		(ns.bulder_.always_full_namespace_ || ns.nm != ns.bulder_.ctx_.nm_cur()) &&
		(ns.nm->parent() != nullptr && !ns.nm->parent()->name().empty())
		) {
		os << ns.nm->to_ts_namespace() << ".";
	}
	return os;
}

static std::string_view fundamental_to_ts(TokenId id) {
	using namespace std::string_view_literals;
	switch (id) {
	case TokenId::Boolean:
		return "boolean"sv;
	case TokenId::Int8:
	case TokenId::UInt8:
	case TokenId::Int16:
	case TokenId::UInt16:
	case TokenId::Int32:
	case TokenId::UInt32:
	case TokenId::Float32:
	case TokenId::Float64: 
		return "number"sv;
	case TokenId::Int64:
	case TokenId::UInt64: 
		return "bigint"sv;
	default: 
		assert(false); 
		return ""sv;
	}
}


void TypescriptBuilder::emit_type(AstTypeDecl* type, std::ostream& os) {
	switch (type->id) {
	case FieldType::Fundamental:
		os << toktype << fundamental_to_ts(cft(type)->token_id) << "/*" << cft(type)->token_id << "*/";
		break;
	case FieldType::Struct:
		os << cflat(type)->name;
		break;
	case FieldType::Vector:
	case FieldType::Array:
		os << "Array<";  emit_type(cwt(type)->type, os); os << ">";
		break;
	case FieldType::String:
		os << "string";
		break;
	case FieldType::Void:
		os << "void";
		break;
	case FieldType::Object:
		os << "NPRPC.detail.ObjectId";
		break;
	case FieldType::Alias:
		os << ns(calias(type)->nm) << calias(type)->name;
		break;
	case FieldType::Enum:
		os << ns(cenum(type)->nm) << cenum(type)->name;
		break;
	case FieldType::Optional:
		emit_type(cwt(type)->type, os);
		break;
	default:
		assert(false);
	}
}

void TypescriptBuilder::emit_flat_type(AstTypeDecl* type, std::ostream& os) {
	switch (type->id) {
	case FieldType::Fundamental:
		os << cft(type)->token_id;
		break;
	case FieldType::Struct: {
		auto s = cflat(type);
		os << ns(s->nm) << "Flat_" << ctx_.current_file() << '.' << s->name;
		break;
	}
	case FieldType::Vector:
		assert(false);
		break;
	case FieldType::String:
		assert(false);
		break;
	case FieldType::Array:
		assert(false);
		break;
	case FieldType::Optional:
		assert(false);
		break;
	case FieldType::Object:
		os << "NPRPC.detail.Flat_nprpc_base.ObjectId";
		break;
	case FieldType::Alias: 
		emit_flat_type(calias(type)->get_real_type(), os);
		break;
	case FieldType::Enum:
		os << ns(cenum(type)->nm) << cenum(type)->name;
		break;
	default:
		assert(false);
	}
}

void TypescriptBuilder::emit_accessors(const std::string& flat_name, AstFieldDecl* f, int& last_field_ended) {
	switch (f->type->id) {
	case FieldType::Fundamental: 
	case FieldType::Enum:
	{
		const int size = get_fundamental_size(cft(f->type)->token_id);
		const int offset = align_offset(size, last_field_ended, size);

		out << "  public get " << f->name <<
			"() { return " << read_field(offset) << cft(f->type)->token_id << "; }\n";

		if (f->type->id == FieldType::Fundamental) {
			out << "  public set " << f->name <<
			"(value: " << fundamental_to_ts(cft(f->type)->token_id) << ") { " << write_field(offset) << cft(f->type)->token_id << "; }\n";
		} else {
			out << "  public set " << f->name <<
			"(value: " <<  ns(cenum(f->type)->nm) << cenum(f->type)->name << ") { " << write_field(offset) << cft(f->type)->token_id << "; }\n";
		}
		
		break;
	}
	case FieldType::Struct: 
		out << "  public get " << f->name << "() { return new " << cflat(f->type)->name
			<< "_Direct(this.buffer, this.offset + " 
			<< align_offset(cflat(f->type)->align, last_field_ended, cflat(f->type)->size) << "); }\n";
		break;

	case FieldType::Vector: {
		auto wt = cwt(f->type)->real_type();
		auto [v_size, v_align] = get_type_size_align(f->type);
		auto [ut_size, ut_align] = get_type_size_align(wt);
		auto const vec_offset = align_offset(v_align, last_field_ended, v_size);

		out <<
			"  public " << f->name << "(elements_size: number): void { \n"
			"    NPRPC.Flat._alloc(this.buffer, this.offset + " << vec_offset << ", elements_size, " << ut_size << ", " << ut_align << ");\n"
			"  }\n";

		if (wt->id == FieldType::Fundamental || wt->id == FieldType::Enum) {
			out <<
				"  public " << f->name << "_d() { return new NPRPC.Flat.Vector_Direct1_" << toktype << cft(wt)->token_id <<
				"(this.buffer, this.offset + " << vec_offset << "); }\n";
		} else if (wt->id == FieldType::Struct) {
			out << "  public " << f->name << "_d() { return new NPRPC.Flat.Vector_Direct2<"; emit_flat_type(wt, out); out << "_Direct>"
				"(this.buffer, this.offset + " << vec_offset << ", " << ut_size << ", "; emit_flat_type(wt, out); out << "_Direct); }\n";
		}

		break;
	}
	case FieldType::Array: {
		auto wt = cwt(f->type)->real_type();
		auto [v_size, v_align] = get_type_size_align(f->type);
		auto [ut_size, ut_align] = get_type_size_align(wt);
		auto const vec_offset = align_offset(v_align, last_field_ended, v_size);
		auto const arr_length = car(f->type)->length;

		if (wt->id == FieldType::Fundamental || wt->id == FieldType::Enum) {
			out << "  public " << f->name << "_d() { return new NPRPC.Flat.Array_Direct1_" << toktype << cft(wt)->token_id <<
				"(this.buffer, this.offset + " << vec_offset << ", " << arr_length <<"); }\n";
		} else if (wt->id == FieldType::Struct) {
			out << "  public " << f->name << "_d() { return new NPRPC.Flat.Array_Direct2<"; emit_flat_type(wt, out); out << "_Direct>"
				"(this.buffer, this.offset + " << vec_offset << ", " << ut_size << ", "; emit_flat_type(wt, out); out << "_Direct" << ", " << arr_length << "); }\n";
		} 
		
		break;
	}
	case FieldType::Optional: {
		auto wt = cwt(f->type)->real_type();
		auto [v_size, v_align] = get_type_size_align(f->type);
		auto [ut_size, ut_align] = get_type_size_align(wt);
		auto const optional_offset = align_offset(v_align, last_field_ended, v_size);

		if (wt->id == FieldType::Fundamental || wt->id == FieldType::Enum) {
			out <<
				"  public " << f->name << "() {\n"
				"    return new NPRPC.Flat.Optional_Direct1_" << toktype << cft(wt)->token_id <<
				"(this.buffer, this.offset + " << optional_offset << ", " << ut_size << ", " << ut_align <<");\n"
				"  }\n";
		} else if (wt->id == FieldType::Struct) {
			out <<
				"  public " << f->name << "() {\n"
				"    return new NPRPC.Flat.Optional_Direct2<"; emit_flat_type(wt, out); out << "_Direct>"
				"(this.buffer, this.offset + " << optional_offset << ", " << ut_size << ", " << ut_align << ", "; emit_flat_type(wt, out); out << "_Direct);\n"
				"  }\n";
		}
		
		break;
	}
	case FieldType::String: {
		const int offset_addr = align_offset(4, last_field_ended, 8);
		// const int n_addr = align_offset(4, last_field_ended, 4);

		out <<
			"  public get " << f->name << "() {\n"
			"    const offset = this.offset + " << offset_addr << ";\n"
			"    const n = this.buffer.dv.getUint32(offset + 4, true);\n"
			"    return n > 0 ? u8dec.decode(new DataView(this.buffer.array_buffer, offset + this.buffer.dv.getUint32(offset, true), n)) : \"\"\n"
			"  }\n";

		out <<
			"  public set " << f->name << "(str: string) {\n"
			"    const bytes = u8enc.encode(str);\n"
			"    const offset = NPRPC.Flat._alloc(this.buffer, this.offset + " << offset_addr << ", bytes.length, 1, 1);\n"
			"    new Uint8Array(this.buffer.array_buffer, offset).set(bytes);\n"
			"  }\n";

		break;
	}
	case FieldType::Object: 
		out <<
			"  public get " << f->name << "() { return "
			"new NPRPC.detail.Flat_nprpc_base.ObjectId_Direct(this.buffer, "
			"this.offset + " << align_offset(align_of_object, last_field_ended, size_of_object) << "); }\n";
		break;
	case FieldType::Alias: {
		auto temp = std::make_unique<AstFieldDecl>();
		temp->name = f->name;
		temp->type = calias(f->type)->get_real_type();
		emit_accessors(flat_name, temp.get(), last_field_ended);
		break;
	}
	default:
		assert(false);
		break;
	}
}

void TypescriptBuilder::emit_parameter_type_for_servant_callback_r(AstTypeDecl* type, std::ostream& os, const bool input) {
	switch (type->id) {
	case FieldType::Fundamental:
		os << fundamental_to_ts(cft(type)->token_id);
		break;
	case FieldType::Struct:
		emit_flat_type(type, os); os << "_Direct";
		break;
	case FieldType::Array: {
		auto wt = cwt(type)->real_type();
		if (wt->id == FieldType::Fundamental || wt->id == FieldType::Enum) {
			out << "typeof NPRPC.Flat.Array_Direct1_" << toktype << cft(wt)->token_id;
		} else if (wt->id == FieldType::Struct) {
			out << "NPRPC.Flat.Array_Direct2<"; emit_parameter_type_for_servant_callback_r(wt, out, input); out << ">";
		}
		break;
	}
	case FieldType::Vector: {
		auto wt = cwt(type)->real_type();
		if (wt->id == FieldType::Fundamental || wt->id == FieldType::Enum) {
			out << "typeof NPRPC.Flat.Vector_Direct1_" << toktype << cft(wt)->token_id;
		} else if (wt->id == FieldType::Struct) {
			out << "NPRPC.Flat.Vector_Direct2<"; emit_parameter_type_for_servant_callback_r(wt, out, input); out << ">";
		}
		break;
	}
	case FieldType::Optional: {
		auto wt = cwt(type)->real_type();
		if (wt->id == FieldType::Fundamental || wt->id == FieldType::Enum) {
			out << "typeof NPRPC.Flat.Optional_Direct1_" << toktype << cft(wt)->token_id;
		} else if (wt->id == FieldType::Struct) {
			out << "NPRPC.Flat.Optional_Direct2<"; emit_parameter_type_for_servant_callback_r(wt, out, input); out << ">";
		}
		break;
	}
	case FieldType::String:
		if (input) {
			os << "string";
		} else {
			os << "NPRPC.String_Direct1";
		}
		break;
	case FieldType::Object:
		if (input) {
			os << "NPRPC.ObjectProxy";
		} else {
			os << "NPRPC.detail.Flat_nprpc_base.ObjectId_Direct";
		}
		break;
	case FieldType::Enum:
		os << ns(cenum(type)->nm) << cenum(type)->name;
		break;
	case FieldType::Alias: {
		if (!input) {
			os << ns(calias(type)->nm) << calias(type)->name;
		} else {
			auto rt = calias(type)->get_real_type();
			if (rt->id == FieldType::Fundamental) {
				os << ns(calias(type)->nm) << calias(type)->name;
			} else {
				emit_parameter_type_for_servant_callback_r(calias(type)->get_real_type(), os, input);
			}
		}
		break;
	}
	case FieldType::Interface:
		assert(false);
		break;
	default:
		assert(false);
		break;
	}
}

void TypescriptBuilder::emit_parameter_type_for_servant_callback(AstFunctionArgument* arg, std::ostream& os) {
	auto const input = (arg->modifier == ArgumentModifier::In);
	emit_parameter_type_for_servant_callback_r(arg->type, os, input);
//	if (!input &&
//		(arg->type->id != FieldType::Vector
//			&& arg->type->id != FieldType::Object
//			&& arg->type->id != FieldType::String)) {
//		os << '&';
//	}
}


void TypescriptBuilder::emit_parameter_type_for_proxy_call_r(AstTypeDecl* type, std::ostream& os, bool input) {
	switch (type->id) {
	case FieldType::Fundamental:
		os << fundamental_to_ts(cft(type)->token_id);
		break;
	case FieldType::Struct:
		os << ns(ctx_.nm_cur()) << cflat(type)->name;
		break;
	case FieldType::Array:
	case FieldType::Vector:
		os << "Array<";
		emit_parameter_type_for_proxy_call_r(cwt(type)->type, os, input);
		os << ">";
		if (type->id == FieldType::Array) os << "/*" << car(type)->length << "*/";
		break;
	case FieldType::String:
		os << "string";
		break;
	case FieldType::Optional:
		emit_parameter_type_for_proxy_call_r(copt(type)->type, os, input);
		break;
	case FieldType::Enum:
		os << ns(cenum(type)->nm) << cenum(type)->name;
		break;
	case FieldType::Alias:
		os << ns(calias(type)->nm) << calias(type)->name;
		break;
	case FieldType::Void:
		os << "void";
		break;
	case FieldType::Object:
		if (input) os << "NPRPC.detail.ObjectId";
		else os << "NPRPC.ref<NPRPC.ObjectProxy>";
		break;
	default:
		assert(false);
	}
}

void TypescriptBuilder::emit_parameter_type_for_proxy_call(AstFunctionArgument* arg, std::ostream& os) {
	const bool input = (arg->modifier == ArgumentModifier::In);

	os << (input ? "/*in*/" : "/*out*/");
//	if (input && arg->type->id != FieldType::Fundamental && arg->type->id != FieldType::Vector) {
//		os << "const ";
//	}
	const bool as_reference = (arg->modifier == ArgumentModifier::Out && (arg->direct || is_fundamental(arg->type)));
	if (as_reference) os << "NPRPC.ref<";
	if (!input && arg->direct) {
		emit_parameter_type_for_servant_callback_r(arg->type, os, input);
	} else {
		emit_parameter_type_for_proxy_call_r(arg->type, os, input);
	}
	if (as_reference) os << '>';
	//if (!input) {
	//	os << '&';
	//}
}


void TypescriptBuilder::assign_from_ts_type(AstTypeDecl* type, std::string op1, std::string op2, bool from_iterator) {
	switch (type->id) {
	case FieldType::Fundamental:
	case FieldType::String:
	case FieldType::Enum:
		out << "  " << op1 << " = " << op2 << ";\n";
		break;
	case FieldType::Struct: {
		auto s = cflat(type);
		for (auto field : s->fields) {
			assign_from_ts_type(field->type, op1 + (from_iterator ? "." : ".") + field->name, op2 + '.' + field->name);
		}
		break;
	}
	case FieldType::Vector:
		out << "  " << op1 << '(' << op2 << ".length);\n";
		[[fallthrough]];
	case FieldType::Array: {
		auto wt = cwt(type)->real_type();
		if (is_fundamental(wt)) {
			// auto [size, align] = get_type_size_align(wt);
			out << "  " << op1 << "_d().copy_from_ts_array(" << op2 << "); \n";
		} else {
			out <<
				"  {\n"
				"  let vv = " << op1 << "_d(), index = 0;\n"
				"  for (let e of vv) {\n"
				"    "; assign_from_ts_type(wt, "    e", op2 + "[index]", true); out <<
				"    ++index;\n"
				"  }\n"
				"  }\n"
				;
		}
		break;
	}
	case FieldType::Optional: {
		auto wt = cwt(type)->real_type();
		if (is_fundamental(wt)) {
			// auto [size, align] = get_type_size_align(wt);
			out << 
				"  {\n"
				"    let opt = " << op1 << "();\n"
				"    if (" << op2 <<") {\n"
				"      opt.alloc();\n"
				"      opt.value = " << op2 << "\n"
				"    } else {\n"
				"      opt.set_nullopt();\n"
				"    }\n"
				"  }\n"
				;
		} else {
			out <<
				"  {\n"
				"    let opt = " << op1 << "();\n"
				"    if (" << op2 << ") {\n"
				"      let opt = " << op1 << "();\n"
				"      opt.alloc();\n"
				"      let value = opt.value;\n"
				"      "; assign_from_ts_type(wt, "value", op2, true); out <<
				"    } else {\n"
				"      opt.set_nullopt();\n"
				"    }\n"
				"  }\n"
				;
		}
		break;
	}
	case FieldType::Alias:
		assign_from_ts_type(calias(type)->type, op1, op2, from_iterator);
		break;
	case FieldType::Object:
		out << "  NPRPC.oid_assign_from_ts(" << op1 << ", " << op2 << ");\n";
		break;
	default:
		assert(false);
		break;
	}
}

void TypescriptBuilder::assign_from_flat_type(
	AstTypeDecl* type,
	std::string op1,
	std::string op2,
	bool from_iterator,
	bool top_object,
	bool direct)
{
	static int _idx = 0;
	switch (type->id) {
	case FieldType::Fundamental:
	case FieldType::String:
	case FieldType::Enum:
		out << "  " << op1 << " = " << op2 << ";\n";
		break;
	case FieldType::Struct: {
		auto s = cflat(type);
		out << "  (" << op1 << " as any) = new Object();\n";
		for (auto field : s->fields) {
			assign_from_flat_type(field->type, op1 + (from_iterator ? "." : ".") + field->name, op2 + '.' + field->name, false, false);
		}
		break;
	}
	case FieldType::Array:
	case FieldType::Vector: {
		auto wt = cwt(type)->real_type();

		if (top_object && direct) {
			// expecting out passed by reference
			out << "  " << op1 << ".value = " << op2 << "_d();\n";
			break;
		}

		auto idxs = "index_" + std::to_string(_idx++);
		out <<
			"  {\n"
			"  let vv = " << op2 << "_d(), " << idxs << " = 0;\n";
		if (top_object) out << "  " << op1 << ".length = vv.elements_size;\n";
		else out << "  (" << op1 << " as Array<any>) = new Array<any>(vv.elements_size)\n";
		out <<
			"  for (let e of vv) {\n";
		if (!is_fundamental(wt)) out << "    (" << op1 << '[' << idxs << "] as any) = new Object();\n";
		assign_from_flat_type(wt, op1 + '[' + idxs + ']', "e", true, false);
		out <<
			"    ++" << idxs << ";\n"
			"  }\n"
			"  }\n"
			;
		break;
	}
	case FieldType::Optional: {
		auto wt = cwt(type)->real_type();
		if (is_fundamental(wt)) {
			// auto [size, align] = get_type_size_align(wt);
			out <<
				"  {\n"
				"    if (" << op2 << ".has_value) {\n"
				"      " << op1 << " = " << op2 << ".value\n"
				"    } else {\n"
				"      " << op1 << " = null\n"
				"    }\n"
				"  }\n"
				;
		} else {
			out <<
				"  {\n"
				"    let opt = " << op2 << "();\n"
				"    if (opt.has_value) {\n"
				"      let value = opt.value;\n"
				"      "; assign_from_flat_type(wt, op1, "value", true); out <<
				"    } else {\n"
				"      " << op1 << " = null\n"
				"    }\n"
				"  }\n"
				;
		}
		break;
	}
	case FieldType::Alias:
		assign_from_flat_type(calias(type)->get_real_type(), op1, op2, from_iterator, false);
		break;
	case FieldType::Object:
		if (top_object) {
			// expecting out passed by reference
			out << "  " << op1 << ".value = NPRPC.create_object_from_flat(" << op2 << ", this.endpoint);\n";
		} else {
			out << "  " << op1 << " = NPRPC.oid_create_from_flat(" << op2 << ");\n";
		}
		break;
	default:
		assert(false);
		break;
	}
}

void TypescriptBuilder::emit_struct2(AstStructDecl* s, bool is_exception) {
	calc_struct_size_align(s);

	// native typescript
	if (!is_exception) {
		out << "export interface " << s->name << " {\n";
		for (auto const f : s->fields) {
			out << "  " << f->name << (!f->is_optional() ? ": " : "?: "); emit_type(f->type, out); out << ";\n";
		}
	} else {
		out <<
			"export class " << s->name << " extends NPRPC.Exception {\n"
			"  constructor(";
		for (size_t ix = 1; ix < s->fields.size(); ++ix) {
			auto f = s->fields[ix];
			out << "public " << f->name << "?: "; emit_type(f->type, out);
			if (ix + 1 < s->fields.size()) out << ", ";
		}
		out << ") { super(\""<< s->name << "\"); }\n";
	}

	out << "}\n\n";

	// flat type

	auto const accessor_name = s->name + "_Direct";

	out << "export namespace Flat_" << ctx_.current_file() << " {\n";
	out << "export class " << accessor_name << " extends NPRPC.Flat.Flat {\n";

	int offset = 0;
	for (auto& f : s->fields) {
		emit_accessors(s->name, f, offset);
	}

	out << "}\n} // namespace Flat \n";
}

void TypescriptBuilder::emit_constant(const std::string& name, AstNumber* number) {
	out << "export const " << name << " = ";
	std::visit(overloaded{
	[&](int64_t x) { 
		out << x;
	},
	[&](float x) { out << x; },
	[&](double x) { out << x; },
	[&](bool x) { out << std::ios::boolalpha << x << std::ios::dec; },
	}, number->value);
	out << ";\n";
}

void TypescriptBuilder::emit_struct(AstStructDecl* s) {
	emit_struct2(s, false);
}

void TypescriptBuilder::emit_exception(AstStructDecl* s) {
	assert(s->is_exception());
	emit_struct2(s, true);
}

void TypescriptBuilder::emit_file_footer() {
	// throw_exception function body
	auto& exs = ctx_.exceptions;
	if (!exs.empty()) {

		out <<
			"\n"

			"function " << ctx_.current_file() << "_throw_exception(buf: NPRPC.FlatBuffer): void { \n"
			"  switch( buf.read_exception_number() ) {\n"
			;

		always_full_namespace(true);
		for (auto ex : exs) {
			out <<
				"  case " << ex->exception_id << ":\n"
				"  {\n"
				"    let ex_flat = new Flat_" << ctx_.current_file() << '.' << ns(ex->nm) << ex->name << "_Direct(buf, " << size_of_header << ");\n"
				"    let ex = new " << ns(ex->nm) << ex->name << "();\n"
				;

			for (size_t i = 1; i < ex->fields.size(); ++i) {
				auto f = ex->fields[i];
				assign_from_flat_type(f->type, "ex." + f->name, "ex_flat." + f->name);
			}


			out <<
				"    throw ex;\n"
				"  }\n"
				;
		}
		always_full_namespace(false);


		out <<
			"  default:\n"
			"    throw \"unknown rpc exception\";\n"
			"  }\n"
			"}\n"
			;
	}

	// other

	emit_arguments_structs(std::bind(&TypescriptBuilder::emit_struct2, this, _1, false));
	emit_struct_helpers();
}

void TypescriptBuilder::emit_using(AstAliasDecl* u) {
	out << "export type " << u->name << " = "; emit_type(u->type, out); out << ";\n";
}

void TypescriptBuilder::emit_enum(AstEnumDecl* e) {
	out << "export const enum " << e->name << " { //" << toktype << e->token_id << '\n';
	std::int64_t ix = 0;
	for (size_t i = 0; i < e->items.size(); ++i) {
		out << "  " << e->items[i].first;
		auto const n = e->items[i].second;
		if (n.second || ix != n.first) { // explicit
			out << " = " << n.first;
			ix = n.first.decimal() + 1;
		} else {
			++ix;
		}
		if (i != e->items.size() - 1) out << ",\n";
	}
	out << "\n}\n";
}

void TypescriptBuilder::emit_namespace_begin() {
	if (ctx_.nm_cur()->parent() && ctx_.nm_cur()->parent()->name().empty()) return;
	out << "export namespace " << ctx_.nm_cur()->name() << " { \n";
}

void TypescriptBuilder::emit_namespace_end() {
	if (ctx_.nm_cur()->parent() && ctx_.nm_cur()->parent()->name().empty()) return;
	out << "} // namespace " << ctx_.nm_cur()->name() << "\n\n";
}

void TypescriptBuilder::emit_interface(AstInterfaceDecl* ifs) {
	auto const flat_nm = "Flat_" + ctx_.current_file();
	const auto servant_iname = 'I' + ifs->name + "_Servant";

	auto emit_function_arguments = [](bool ts, AstFunctionDecl* fn, std::ostream& os,
		std::function<void(AstFunctionArgument*, std::ostream& os)> emitter) {
			os << "(";
			size_t ix = 0;
			for (auto arg : fn->args) {
				os << arg->name;
				if (!ts) os << ": ";
				else os << (!arg->is_optional() ? ": " : "?: ");
				emitter(arg, os);
				if (++ix != fn->args.size()) os << ", ";
			}
			os << ')';
	};


	// Proxy definition =======================================================
	out <<
		"export class " << ifs->name;

	//if (ifs->plist.size()) {
		//out << " extends " << ifs->plist[0]->name << "\n";
		//for (size_t i = 1; i < ifs->plist.size(); ++i) {
		//	out << " extends " << ifs->plist[i]->name << "\n";
		//}
		//out << "{\n";
	//} else {
	//}

	out << 
		" extends NPRPC.ObjectProxy {\n"
		"  public static get servant_t(): new() => _"<< servant_iname <<" {\n"
		"    return _" << servant_iname << ";\n"
		"  }\n\n"
		;

	// parent's functions
	std::map<AstInterfaceDecl*, int> ifs_idxs;
	auto count_all = [&ifs_idxs](AstInterfaceDecl* ifs_inherited, int& n) { 
		ifs_idxs.emplace(ifs_inherited, n);
	};

	int n = 1;
	for (auto parent : ifs->plist) {
		dfs_interface(std::bind(count_all, _1, std::ref(n)), parent);
	}
		
	for (auto& inherited_ifs : ifs_idxs) {
		if (inherited_ifs.first->fns.size()) {
			out << "  // " << inherited_ifs.first->name << '\n';
		}
		for (auto& fn : inherited_ifs.first->fns) {
			out << "  public async " << fn->name;
			emit_function_arguments(false, fn, out,
				std::bind(&TypescriptBuilder::emit_parameter_type_for_proxy_call, this, _1, _2)
			);
			out << ": Promise<"; emit_type(fn->ret_value, out);  out << "> {\n";
			out << "    " << (!fn->is_void() ? "return " : "") << inherited_ifs.first->name << ".prototype." << fn->name << ".bind(this,";
			for (auto arg : fn->args) {
				out << arg->name << ',';
			}
			out << inherited_ifs.second << ")();\n  }\n";
		}
	}
	// proxy object functions definitions
	out << '\n';
	for (auto& fn : ifs->fns) {
		make_arguments_structs(fn);
		out << "  public async " << fn->name;
		emit_function_arguments(true, fn, out,
			std::bind(&TypescriptBuilder::emit_parameter_type_for_proxy_call, this, _1, _2)
		);
		out << ": Promise<"; emit_type(fn->ret_value, out);  out << "> {\n"
			"    let interface_idx = (arguments.length == " << fn->args.size() << " ? 0 : arguments[arguments.length - 1]);\n"
			;

		const auto fixed_size = get_arguments_offset() + (fn->in_s ? fn->in_s->size : 0);
		const auto capacity = fixed_size + (fn->in_s ? (fn->in_s->flat ? 0 : 128) : 0);
		out << 
			"    let buf = NPRPC.FlatBuffer.create();\n"
			"    buf.prepare(" << capacity << ");\n"
			"    buf.commit(" << fixed_size << ");\n"
			"    buf.write_msg_id(NPRPC.impl.MessageId.FunctionCall);\n"
			"    buf.write_msg_type(NPRPC.impl.MessageType.Request);\n"
			"    let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, " << size_of_header << ");\n"
			"    __ch.object_id = this.data.object_id;\n"
			"    __ch.poa_idx = this.data.poa_idx;\n"
			"    __ch.interface_idx = interface_idx;\n"
			"    __ch.function_idx = " << fn->idx << ";\n"
			;

		if (fn->in_s) {
			out <<
				"  let _ = new Flat_" << ctx_.current_file() << '.' << fn->in_s->name << "_Direct(buf," << get_arguments_offset() << ");\n"
				;
		}

		int ix = 0;
		for (auto in : fn->args) {
			if (in->modifier == ArgumentModifier::Out) continue;
			assign_from_ts_type(in->type, "_._" + std::to_string(++ix), in->name);
		}

		out << 
			"    buf.write_len(buf.size - 4);\n";

		out <<
			"    await NPRPC.rpc.call(\n"
			"      this.endpoint, buf, this.timeout\n"
			"    );\n"
			"    let std_reply = NPRPC.handle_standart_reply(buf);\n"
			;

		if (fn->ex) {
			out <<
				"    if (std_reply == 1) {\n"
				"      " << ctx_.current_file() << "_throw_exception(buf);\n"
				"    }\n"
				;
		}

		if (!fn->out_s) {
			out <<
				"    if (std_reply != 0) {\n"
				"      console.log(\"received an unusual reply for function with no output arguments\");\n"
				"    }\n"
				;
		} else {
			out <<
				"    if (std_reply != -1) {\n"
				"      console.log(\"received an unusual reply for function with output arguments\");\n"
				"      throw new NPRPC.Exception(\"Unknown Error\");\n"
				"    }\n"
				;

			// (sizeof the message + sizeof BlockResponse) next struct will be always aligned by 8
			out << "  let out = new Flat_" << ctx_.current_file() << '.' << fn->out_s->name << "_Direct(buf, " << size_of_header << ");\n";

			int ix = fn->is_void() ? 0 : 1;

			for (auto out : fn->args) {
				if (out->modifier == ArgumentModifier::In) continue;
				assign_from_flat_type(out->type, out->name + (is_fundamental(out->type) ? ".value" : ""), "out._" + std::to_string(++ix), false, true, out->direct);
			}

			if (!fn->is_void()) {
				out << "  let __ret_value: "; emit_type(fn->ret_value, out); out << ";\n";
				assign_from_flat_type(fn->ret_value, "__ret_value", "out._1");
				out << "  return __ret_value;\n";
			}
		}

		out << "}\n\n";
	}

	out << "};\n\n"; // Proxy class ends
	
	// Servant definition
	out << "export interface " << servant_iname << '\n';
	if (ifs->plist.size()) {
		out << " extends I" << ifs->plist[0]->name << "_Servant\n";
		for (size_t i = 1; i < ifs->plist.size(); ++i) {
			out << ", I" << ifs->plist[i]->name << "_Servant\n";
		}
	}
	out << "{\n";

	for (auto fn : ifs->fns) {
		out << "  " << fn->name;
		emit_function_arguments(false, fn, out,
			std::bind(&TypescriptBuilder::emit_parameter_type_for_servant_callback, this, _1, _2)
		);
		out << ": "; emit_type(fn->ret_value, out); out << ";\n";
	}

	out << "}\n\n"; // interface ends

	out <<
		"export class _" << servant_iname << " extends NPRPC.ObjectServant {\n"
		"  public static _get_class(): string { return \"" << ctx_.current_file() << '/' << ctx_.nm_cur()->to_ts_namespace(false) << '.' /*ns(ctx_.nm_cur()) */ << ifs->name << "\"; }\n"
		"  public readonly get_class = () => { return _"<< servant_iname << "._get_class(); }\n"
		"  public readonly dispatch = (buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean) => {\n"
		"    _" << servant_iname << "._dispatch(this, buf, remote_endpoint, from_parent);\n"
		"  }\n"
		"  static _dispatch(obj: _" << servant_iname << ", buf: NPRPC.FlatBuffer, remote_endpoint: NPRPC.EndPoint, from_parent: boolean): void {\n"
		;

	// Servant dispatch ====================================================
	out << 
		"  let __ch = new NPRPC.impl.Flat_nprpc_base.CallHeader_Direct(buf, " << size_of_header << ");\n"
		;
		
	if (ifs->plist.empty()) {
		// ok
	} else {
		out <<
			"  if (from_parent == false) {\n"
			"    switch(__ch.interface_idx) {\n"
			"      case 0:\n"
			"        break;\n"
			;
	
		int ix = 1;
		auto select_interface = [&ix, this, ifs](AstInterfaceDecl* i) {
			if (i == ifs) return;
			out <<
				"      case "<< ix << ":\n"
				"        _I" << i->name << "_Servant._dispatch(obj, buf, remote_endpoint, true);\n"
				"        return;\n"
			;
			++ix;
		};
		
		dfs_interface(select_interface, ifs);
		
		out <<
			"      default:\n"
			//"        assert(false);\n"
			"        throw \"unknown interface\";\n"
			"    }\n"
			"  }\n"
			;
	}

	out <<
			"  switch(__ch.function_idx) {\n"
			;


	for (auto fn : ifs->fns) {
		out << "    case " << fn->idx << ": {\n";

		int out_ix = fn->is_void() ? 0 : 1;

		if (fn->out_s && fn->out_s->flat) {
			for (auto arg : fn->args) {
				if (arg->modifier == ArgumentModifier::Out) {
					out << "      let _out_" << ++out_ix << ": "; emit_type(arg->type, out); out << ";\n";
				}
			}
		}

		if (fn->in_s) {
			out <<
				"      let ia = new " << flat_nm << '.' << fn->in_s->name << "_Direct(buf, " << get_arguments_offset() << ");\n"
				;
		}

		if (fn->out_s && !fn->out_s->flat) {
			const auto offset = size_of_header;
			const auto initial_size = offset + fn->out_s->size;
			out <<
				"      let obuf = buf;\n"
				"      obuf.consume(obuf.size);\n"
				"      obuf.prepare(" << initial_size + 128 << ");\n"
				"      obuf.commit(" << initial_size << ");\n"
				"      let oa = new " << flat_nm << '.' << fn->out_s->name << "_Direct(obuf," << offset << ");\n"
				;
		}

		if (!fn->is_void()) {
			out << "let __ret_val: "; emit_type(fn->ret_value, out); out << ";\n";
		}

		if (fn->ex) out << "      try {\n";
		
		out <<
			(fn->is_void() ? "      " : "      __ret_val = ") << "(obj as any)." << fn->name << "("
			;

		size_t in_ix = 0, idx = 0; out_ix = fn->is_void() ? 0 : 1;
		for (auto arg : fn->args) {
			if (arg->modifier == ArgumentModifier::Out) {
				assert(fn->out_s);
				if (!fn->out_s->flat) {
					out << "oa._" << ++out_ix;
					if (arg->type->id == FieldType::Vector || arg->type->id == FieldType::Array || arg->type->id == FieldType::String) {
						out << "_d";
					}
				} else {
					out << "_out_" << ++out_ix;
				}
			} else {
				if (arg->type->id == FieldType::Object) {
					out << "NPRPC.create_object_from_flat(ia._" << ++in_ix << ", remote_endpoint)";
				} else {
					out << "ia._" << ++in_ix;
					if (arg->type->id == FieldType::Vector || arg->type->id == FieldType::Array) out << "_d()";
				}
			}
			if (++idx != fn->args.size()) out << ", ";
		}
		out << ");\n";

		/*
		out_ix = fn->is_void() ? 0 : 1;
		for (auto arg : fn->args) {
			if (arg->modifier == ArgumentModifier::Out) {
				++out_ix;
				if (arg->type->id == FieldType::Object) {
					oc <<
						"{\n"
						"  auto obj = impl::g_orb->get_object(" << "oa._" << out_ix << "().poa_idx(), " << "oa._" << out_ix << "().object_id());\n"
						"  if (obj) if (auto real_obj = (*obj).get(); real_obj) ref_list.add_ref(real_obj);\n"
						"}\n"
						;
				}
			}
		}
		*/

		if (fn->ex) {
			const auto offset = size_of_header;
			const auto initial_size = offset + fn->ex->size;
			
			always_full_namespace(true);
			out <<
				"      }\n"
				"      catch(e) {\n"
				"        let obuf = buf;\n"
				"        obuf.consume(obuf.size);\n"
				"        obuf.prepare(" << initial_size << ");\n"
				"        obuf.commit(" << initial_size << ");\n"
				"        let oa = new "<< ns(fn->ex->nm) << flat_nm << '.' << fn->ex->name << "_Direct(obuf," << offset << ");\n"
				"        oa.__ex_id = " << fn->ex->exception_id << ";\n"
				;
			always_full_namespace(false);

			for (size_t i = 1; i < fn->ex->fields.size(); ++i) {
				auto mb = fn->ex->fields[i];
				assign_from_ts_type(mb->type, "oa." + mb->name, "e." + mb->name);
			}
	
			out << 
				"        obuf.write_len(obuf.size - 4);\n"
				"        obuf.write_msg_id(NPRPC.impl.MessageId.Exception);\n"
				"        obuf.write_msg_type(NPRPC.impl.MessageType.Answer);\n"
				"        return;\n"
				"      }\n"
				;
		}

		if (!fn->out_s) {
			out << "      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Success);\n";
		} else {
			if (fn->out_s->flat) { // it means that we are writing output data in the input buffer, 
				// so we must pass stack variables first and then assign result back to the buffer
				const auto offset = size_of_header;
				const auto initial_size = offset + fn->out_s->size;

				out <<
					"      let obuf = buf;\n"
					"      obuf.consume(obuf.size);\n"
					"      obuf.prepare(" << initial_size << ");\n"
					"      obuf.commit(" << initial_size << ");\n"
					"      let oa = new " << flat_nm << '.' << fn->out_s->name << "_Direct(obuf," << offset << ");\n"
					;
				
				int ix;
				if (!fn->is_void()) {
					assign_from_ts_type(fn->ret_value, "oa._1", "__ret_val");
					ix = 1;
				} else {
					ix = 0;
				}

				for (auto out : fn->args) {
					if (out->modifier == ArgumentModifier::In) continue;
					auto n = std::to_string(++ix);
					assign_from_ts_type(out->type, "oa._" + n, "_out_" + n);
				}
			} else if (!fn->is_void()) {
				assign_from_ts_type(fn->ret_value, "oa._1", "__ret_val");
			}

			out <<
				"      obuf.write_len(obuf.size - 4);\n"
				"      obuf.write_msg_id(NPRPC.impl.MessageId.BlockResponse);\n"
				"      obuf.write_msg_type(NPRPC.impl.MessageType.Answer);\n"
				;
		}

		out <<
			"      break;\n"
			"    }\n"
			;
	}

	out <<
		"    default:\n"
		"      NPRPC.make_simple_answer(buf, NPRPC.impl.MessageId.Error_UnknownFunctionIdx);\n"
		"  }\n"; // switch block ends
	;


  out <<
		"  }\n" // dispatch ends
		"}\n\n" // class ends
		;
}


void TypescriptBuilder::emit_struct_helpers() {
		for (auto s : ctx_.structs_with_helpers) {
			out << "export namespace " << ns(s->nm) << "helpers {\n";
			auto saved_namespace = ctx_.set_namespace(s->nm);
			// flat -> ts
			out << "export const assign_from_flat_" << s->name << " = (src: ";
			emit_parameter_type_for_servant_callback_r(s, out, false); 
			out << "): ";
			emit_parameter_type_for_proxy_call_r(s, out, false); 
			out << " => {\n  let dest: ";
			emit_parameter_type_for_proxy_call_r(s, out, false); 
			out << ";\n";
			assign_from_flat_type(s, "dest", "src", false, true);
			out << "  return dest;\n}\n";
			// ts -> flat
			out << "export const assign_from_ts_" << s->name << " = (dest: ";
			emit_parameter_type_for_servant_callback_r(s, out, false);
			out << ", src: ";
			emit_parameter_type_for_proxy_call_r(s, out, false); 
			out << ") => {\n";
			assign_from_ts_type(s, "dest", "src");
			out << "}\n";
			ctx_.set_namespace(saved_namespace);
			out << "} // namespace " << s->nm->name() << ".helpers\n";
	}

}

TypescriptBuilder::_ns TypescriptBuilder::ns(Namespace* nm) {
	return { *this, nm };
}

TypescriptBuilder::TypescriptBuilder(Context& ctx, std::filesystem::path file_path, std::filesystem::path out_dir)
	: Builder(ctx)
{
	auto filename = file_path.filename();
	filename.replace_extension(".ts");
	out.open(out_dir / filename, std::ios::binary);

	if (ctx_.is_nprpc_base()) {
		out << "import * as NPRPC from '@/base'\n\n";
	} else if (ctx_.is_nprpc_nameserver()) {
		out << "import * as NPRPC from '@/index_internal'\n\n";
	} else {
		out << "import * as NPRPC from 'nprpc'\n\n";
	}

	out <<
		"const u8enc = new TextEncoder();\n"
		"const u8dec = new TextDecoder();\n\n"
		;

}
