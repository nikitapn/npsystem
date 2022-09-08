// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "memorymanager.h"
#include "block.h"
#include "sfc_block.hpp"
#include "codegenerator_avr.h"
#include "control_unit_ext.h"
#include "library.h"
#include "global.h"
#include "genhelper.h"
#include <avr_firmware/net.h>
#include <boost/log/trivial.hpp>

#define BLOCK_BEGIN(block) \
	auto block_id_str = "bid_" + std::to_string(block->top->self_node.id()); \
	m_code << "\t/* " + block_id_str + " */\n{\n";

#define BLOCK_END(block) \
	m_code << "}\n";

/*
Call-Used Registers
The call-used or call-clobbered general purpose registers (GPRs) are registers that might be destroyed (clobbered) 
by a function call.

R18-R27, R30, R31
These GPRs are call clobbered. An ordinary function may use them without restoring the contents.
Interrupt service routines (ISRs) must save and restore each register they use.

R0, T-Flag
The temporary register and the T-flag in SREG are also call-clobbered, 
but this knowledge is not exposed explicitly to the compiler (R0 is a fixed register).

Call-Saved Registers
R2-R17, R28, R29
The remaining GPRs are call-saved, i.e. a function that uses such a registers must restore its original content.
This applies even if the register is used to pass a function argument.

R1
The zero-register is implicity call-saved (implicit because R1 is a fixed register).
*/

void AsmBlock::AddClobber(int reg_num) {
	//if (!((reg_num >= 18 && reg_num <= 27) || (reg_num >= 30 && reg_num <= 31))) {
		//std::cout << "r" << reg_num << " is not call clobbered" << std::endl;
		// throw "utilizing a not call clobbered register as is not allowed";
	//}
	if (!m_clobbers.empty()) m_clobbers += ',';
	m_clobbers += "\"r" + std::to_string(reg_num) + "\"";
}

void AsmBlock::AddOperand(OP_TYPE type, const std::string& name, const npsys::variable* var) {
	if (type == OP_TYPE::GCC_OP_RW || type == OP_TYPE::GCC_OP_W) {
		m_ops.push_front({ type, name, var });
	} else {
		m_ops.push_back({ type, name, var });
	}
}

std::tuple<std::string, std::string, std::string> AsmBlock::Normalize() const noexcept {
	constexpr int MAX_STACK_SIZE = 50;
	boost::container::small_vector<int, MAX_STACK_SIZE> tab(MAX_STACK_SIZE);
	
	std::string input_v, output_v;
	auto body = m_body.str();

	auto add_input = [&input_v](const std::string& type, const std::string& c) {
		if (!input_v.empty()) input_v += ',';
		input_v += "\"" + type + "\"(" + c + ")";
	};

	auto add_output = [&output_v](const std::string& type, const std::string& c) {
		if (!output_v.empty()) output_v += ',';
		output_v += "\"" + type + "\"(" + c + ")";
	};

	int pos = 0;
	size_t size = m_ops.size();
	if (size > MAX_STACK_SIZE) tab.resize(size);
	for (auto& i : tab) i = 0;
	
	for (int i = 0; i < size; ++i) {
		if (tab[i] == -1) continue;

		const auto& cur = m_ops[i];
		const bool local_variable = (cur.var == nullptr);
		const bool quality_byte = (!local_variable && !cur.var->IsBit() && cur.var->IsQuality());

		std::string c_name;

		if (local_variable) {
			c_name = cur.name.substr(1);
		} else {
			c_name = cur.var->to_vardecl();
		}

		std::string op_name = "%" + std::to_string(pos);

		switch (cur.type) {
		case OP_TYPE::GCC_OP_RW:
			add_output("+r", c_name);
			if (quality_byte) add_output("+r", c_name + "_q");
			break;
		case OP_TYPE::GCC_OP_W:
			add_output("w", c_name);
			if (quality_byte) add_output("w", c_name + "_q");
			break;
		case OP_TYPE::GCC_OP_R:
			add_input("r", c_name);
			if (quality_byte) add_input("r", c_name + "_q");
			break;
		default:
			ASSERT(FALSE);
			break;
		}

		if (quality_byte) {
			std::string op_qname = "%" + std::to_string(++pos);
			std::string op_old_qname = cur.name + "_q";
			boost::replace_all(body, op_old_qname, op_qname);
		}

		boost::replace_all(body, cur.name, op_name);

		if (!local_variable) {
			int addr = cur.var->GetAddr();
			for (int j = i + 1; j < size; ++j) {
				if (tab[j] == -1 || m_ops[j].var == nullptr) continue;
				if (m_ops[j].var->GetAddr() == addr) {
					tab[j] = -1;
					boost::replace_all(body, m_ops[j].name, op_name);
				}
			}
		}
		pos++;
		tab[i] = -1;
	}

	size_t len = body.length();
	for (size_t i = 1; i < len; ++i) {
		if (body[i] != '%') continue;
		if (body[i - 1] >= 'A' && body[i - 1] <= 'D') {
			std::swap(body[i], body[i - 1]);
		}
	}

	return { body, input_v, output_v };
}

std::stringstream& operator << (std::stringstream& ss, const AsmBlock& bl) {
	auto [body, input_v, output_v] = bl.Normalize();
	
	if (body.empty()) return ss;

	ss << "\t__asm__ __volatile__ (\n";
	ss << body;
	ss << "\t\t:" << output_v << "\n";
	ss << "\t\t:" << input_v << "\n";

	if (!bl.m_clobbers.empty()) {
		ss << "\t\t:" << bl.m_clobbers << "\n";
	}

	ss << "\t);\n";
	return ss;
}

CAVR5CodeGenerator::CAVR5CodeGenerator(npsys::fbd_control_unit_n alg, const avrinfo::FirmwareInfo& info)
	: alg_(alg)
	, info_(info)
{
	gcc_ = std::make_unique<CGccWrapperAvr>();
	Reset();
}

void CAVR5CodeGenerator::Reset() {
	m_code.str("");
	m_code.clear();
	m_code << "uint16_t algorithm(void)\n{\n";
}

void CAVR5CodeGenerator::SaveToFile() {
	DWORD sp = static_cast<DWORD>((float)alg_->scan_period / 1000.0f / info_.mccfg.one_tick_time);
	m_code << "\treturn " << sp << ";\n";

	::CreateDirectoryA(global.GetAlgorithmsDir().c_str(), NULL);
	::CreateDirectoryA((global.GetAlgorithmsDir() + std::to_string(alg_->id())).c_str(), NULL);

	std::ofstream alg_code(global.GetAlgorithmsDir() + std::to_string(alg_->id()) + "\\alg.c");
	alg_code << "#define F_CPU 16000000UL\n";
	alg_code << "#include <avr/io.h>\n";
	alg_code << "#include <util/delay.h>\n";
	alg_code << "#include <util/atomic.h>\n";

	for (auto& lib : libraries) {
		if (lib == "avr_libm" || lib == "avr_libgcc") continue;
		alg_code << "#include \"../../libraries/" + lib + "/" + lib + ".h\"\n";
	}

	alg_code << "#define discrete_t uint8_t\n";

/*
from https://gcc.gnu.org/onlinedocs/gcc-4.7.2/gcc/Function-Attributes.html
	OS_main/OS_task
	On AVR, functions with the OS_main or OS_task attribute do not save/restore any call-saved register in their prologue/epilogue.
	The OS_main attribute can be used when there is guarantee that interrupts are disabled at the time when the function is entered. This will save resources when the stack pointer has to be changed to set up a frame for local variables.

	The OS_task attribute can be used when there is no guarantee that interrupts are disabled at that time when the function is entered like for, e.g. task functions in a multi-threading operating system. In that case, changing the stack pointer register will be guarded by save/clear/restore of the global interrupt enable flag.

	The differences to the naked function attribute are:

	naked functions do not have a return instruction whereas OS_main and OS_task functions will have a RET or RETI return instruction.
	naked functions do not set up a frame for local variables or a frame pointer whereas OS_main and OS_task do this as needed.
*/

	alg_code << "__attribute__((section(\".text.algorithm\"))) __attribute__((OS_task)) uint16_t algorithm(void);\n";
	alg_code << "register uint8_t reserved_register_r2 asm(\"r2\") __attribute__((used));\n";
	alg_code << "register uint8_t reserved_register_r3 asm(\"r3\") __attribute__((used));\n";
	alg_code << "static uint32_t dwCnt __attribute__((section(\".dwCnt\"))) __attribute__((__used__));\n";

	alg_code << gcc_->GetVarDecl() + "\n";
	alg_code << m_code.str();
	alg_code << "}\n";
}

void CAVR5CodeGenerator::AddLibrary(std::string lib) {
	npsys::libs.get_by_name(lib);
	libraries.emplace(lib);
}

std::string CAVR5CodeGenerator::CastBit(const npsys::variable* var) noexcept {
	std::string var_name = "cnvb_" + std::to_string(conv_bit_n_++);
	m_code << "uint8_t " << var_name << " = (" <<
		var->to_vardecl() << " >> " << var->GetBit() << ") & 0x01;";
	return var_name;
}

std::string CAVR5CodeGenerator::CastQBit(const npsys::variable* var) noexcept {
	std::string var_name = "cnvb_q_" + std::to_string(conv_bit_n_++);
	m_code << "uint8_t " << var_name << " = (" <<
		var->to_vardecl() << " >> " << var->GetQBit() << ") & 0x01;";
	return var_name;
}

void CAVR5CodeGenerator::Generate(CInput* block) {
	// Nothing to calculate
	block;
}

void CAVR5CodeGenerator::Generate(COutput* block) {
	BLOCK_BEGIN(block);

	const auto slot = static_cast<CInputSlot*>(block->GetSlot());
	
	const auto in = slot->GetInputVariable();
	if (!in) return;
	
	const auto out = slot->GetVariable();
	if (!out) return;

	gcc_->AddVariable(in);
	gcc_->AddVariable(out);

	auto sz_in = in->GetSize();
	auto sz_out = out->GetSize();
	
	auto traits = block->GetSlot()->GetSlotType()->GetParameterTraits();
	auto inverted = traits.IsInverted();

	if (out->IsBit()) {
		auto input_has_quality_bit = in->IsQuality();
		auto output_has_quality_bit = out->IsQuality();

		AsmBlock bl(block_id_str);

		if (input_has_quality_bit) {
			bl.AddCmd("bst $in, ", in->GetQBit());
			bl.AddCmd("brtc ", block_id_str, "_bad_q");
		}

		if (output_has_quality_bit) {
			if (!input_has_quality_bit) {
				bl.AddCmd("set"); // io always have good quality
			}
			bl.AddCmd("bld $out, ", out->GetQBit());
		}

		bl.AddCmd("bst $in, ", in->GetBit());
		if (inverted) {
			bl.AddCmd("BRTS .+4");
			bl.AddCmd("SET");
			bl.AddCmd("RJMP .+2");
			bl.AddCmd("CLT");
		}
		bl.AddCmd("bld $out, ", out->GetBit());

		if (input_has_quality_bit) {
			bl.AddCmd("rjmp ", block_id_str, "_end");
			bl.AddLabel("_bad_q");
			// quality bit T is 0 here
			if (output_has_quality_bit) {
				bl.AddCmd("bld $out, ", out->GetQBit());
			} else {
				if (inverted) bl.AddCmd("set");
				bl.AddCmd("bld $out, ", out->GetBit());
			}
			bl.AddLabel("_end");
		}

		bl.AddOperand(OP_TYPE::GCC_OP_R, "$in", in);
		bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);

		m_code << bl;
	} else {
		if (!in->IsBit()) {
			m_code << out->to_vardecl() << " = " << in->to_vardecl() << ";";
			if (in->IsQuality() && out->IsQuality()) {
				m_code << out->to_vardecl() << "_q = " << in->to_vardecl() << "_q;";
			} else if (!in->IsQuality() && out->IsQuality()) {
				m_code << out->to_vardecl() << "_q = 0xFF;";
			}
		} else {

		}
	}
	BLOCK_END(block);
}

// Logic

void CAVR5CodeGenerator::Generate(COr* block) {
	BLOCK_BEGIN(block);

	auto connected_inputs = block->InputHasValidVariable();
	if (connected_inputs == 0) return;

	AsmBlock bl(block_id_str);

	auto& slots = block->top->slots;
	const auto out = o_at(0).GetVariable();

	gcc_->AddVariable(out);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);

	bl.AddCmd("clr __tmp_reg__");
	bl.AddCmd("inc __tmp_reg__");

	ForIn(slots) {
		const auto in = ins().GetInputVariable();
		if (!in) continue;

		gcc_->AddVariable(in);

		std::string op_name = "$in_" + std::to_string(ix + 1);
		bl.AddOperand(OP_TYPE::GCC_OP_R, op_name, in);

		bool input_has_quality = in->IsQuality();
		if (in->IsBit()) {
			if (input_has_quality) {
				bl.AddCmd("bst ", op_name, ",", in->GetQBit());
				bl.AddCmd("brts .+4");
				bl.AddCmd("bld __tmp_reg__, 0");
				bl.AddCmd("rjmp .+4");
			}
			bl.AddCmd("bst ", op_name, ",", in->GetBit());
			bl.AddCmd("brts ", block_id_str, "_store_one");
		}
	}

	bl.AddCmd("bld $out, ", out->GetBit());
	bl.AddCmd("bst __tmp_reg__, 0");
	bl.AddCmd("bld $out, ", out->GetQBit());
	bl.AddCmd("rjmp ", block_id_str, "_end");

	bl.AddLabel("_store_one");
	bl.AddCmd("bld $out,", out->GetBit());
	bl.AddCmd("bld $out,", out->GetQBit());

	bl.AddLabel("_end");

	m_code << bl;
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CAnd* block) {
	BLOCK_BEGIN(block);
	auto connected_inputs = block->InputHasValidVariable();
	if (connected_inputs == 0) return;

	AsmBlock bl(block_id_str);

	auto& slots = block->top->slots;
	const auto out = o_at(0).GetVariable();

	gcc_->AddVariable(out);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);

	bl.AddCmd("clr __tmp_reg__");
	bl.AddCmd("inc __tmp_reg__");

	ForIn(slots) {
		const auto in = ins().GetInputVariable();
		if (!in) continue;

		gcc_->AddVariable(in);
		std::string op_name = "$in_" + std::to_string(ix + 1);
		bl.AddOperand(OP_TYPE::GCC_OP_R, op_name, in);

		bool input_has_quality = in->IsQuality();

		if (in->IsBit()) {
			if (input_has_quality) {
				bl.AddCmd("bst ", op_name, ",", in->GetQBit());
				bl.AddCmd("brts .+4");
				bl.AddCmd("bld __tmp_reg__, 0");
				bl.AddCmd("rjmp .+4");
			}
			bl.AddCmd("bst ", op_name, ",", in->GetBit());
			bl.AddCmd("brtc ", block_id_str, "_store_zero");
		}
	}

	bl.AddCmd("bld $out, ", out->GetBit());
	bl.AddCmd("bst __tmp_reg__, 0");
	bl.AddCmd("bld $out, ", out->GetQBit());
	bl.AddCmd("rjmp ", block_id_str, "_end");

	bl.AddLabel("_store_zero");
	bl.AddCmd("bld $out, ", out->GetBit());
	bl.AddCmd("set");
	bl.AddCmd("bld $out,", out->GetQBit());

	bl.AddLabel("_end");
	
	m_code << bl;
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CNot* block) {
	BLOCK_BEGIN(block);

	auto& slots = block->top->slots;
	const auto in = i_at(0).GetInputVariable();
	if (!in) return;
	const auto out = o_at(0).GetVariable();

	gcc_->AddVariable(in);
	gcc_->AddVariable(out);

	if (in->IsBit()) {
		AsmBlock bl(block_id_str);
		if (in->IsQuality()) {
			bl.AddCmd("bst $in, ", in->GetQBit());
			bl.AddCmd("brtc ", block_id_str, "_bad_quality");
		}
		bl.AddCmd("bst $in,", in->GetBit());
		bl.AddCmd("BRTS .+4");
		bl.AddCmd("SET");
		bl.AddCmd("RJMP .+2");
		bl.AddCmd("CLT");
		bl.AddCmd("bld $out,", out->GetBit());
		bl.AddCmd("SET");
		bl.AddLabel("_bad_quality");
		bl.AddCmd("bld $out,", out->GetQBit());

		bl.AddOperand(OP_TYPE::GCC_OP_R, "$in", in);
		bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);

		m_code << bl;
	} else if (!(in->GetType() & npsys::nptype::NPT_F32)) {
		// ASM_CONV_INV_BYTE_1_TO_BIT(in->to_vardecl(), out->to_vardecl(), out->GetBit());
	}
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CRsTrigger* block) {
	BLOCK_BEGIN(block);

	auto& slots = block->top->slots;

	const auto set = i_at(0).GetInputVariable();
	const auto reset = i_at(1).GetInputVariable();
	if (!set && !reset) return;

	const auto out = o_at(0).GetVariable();

	AsmBlock bl(block_id_str);
	gcc_->AddVariable(out);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);
	
	int io_count = 0;
	
	if (reset) {
		gcc_->AddVariable(reset);
		bl.AddOperand(OP_TYPE::GCC_OP_R, "$reset", reset);
		
		if (reset->IsQuality()) {
			bl.AddCmd("bst $reset, ", reset->GetQBit());
			bl.AddCmd("brtc ", block_id_str + "_bad_q");
		} else {
			io_count++;
		}
	}
	if (set) {
		gcc_->AddVariable(set);
		bl.AddOperand(OP_TYPE::GCC_OP_R, "$set", set);

		if (set->IsQuality()) {
			bl.AddCmd("bst $set, ", set->GetQBit());
			bl.AddCmd("brtc ", block_id_str, "_bad_q");
		} else {
			io_count++;
		}
	}

	if (reset) {
		bl.AddClobber(24);
		bl.AddCmd("ldi r24, 1 << ", reset->GetBit());
		bl.AddCmd("eor r24, $reset");
		bl.AddCmd("bst r24, ", reset->GetBit());
		bl.AddCmd("BRTC ", block_id_str, "_store");
	}

	if (set) {
		bl.AddCmd("bst $set, ", set->GetBit());
		bl.AddCmd("BRTS ", block_id_str, "_store");
		bl.AddCmd("rjmp ", block_id_str, "_write_q");
	}

	if (io_count != 2) {
		bl.AddLabel("_bad_q");
		bl.AddCmd("bld $out, ", out->GetQBit());
		bl.AddCmd("rjmp ", block_id_str, "_end");
	}

	bl.AddLabel("_store");
	bl.AddCmd("bld $out, ", out->GetBit());

	bl.AddLabel("_write_q");
	bl.AddCmd("set");
	bl.AddCmd("bld $out, ", out->GetQBit());

	if (io_count != 2) {
		bl.AddLabel("_end");
	}

	m_code << bl;
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CPositiveEdge* block) {
	BLOCK_BEGIN(block);
	auto& slots = block->top->slots;

	const auto in = i_at(0).GetInputVariable();
	if (!in) return;

	AsmBlock bl(block_id_str);

	const auto prev_in = block->variable_.get();
	const auto out = o_at(0).GetVariable();

	gcc_->AddVariable(in);
	bl.AddOperand(OP_TYPE::GCC_OP_R, "$in", in);

	gcc_->AddVariable(out);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);

	gcc_->AddVariable(prev_in);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$prev_in", prev_in);

	bl.AddClobber(24);

	if (in->IsQuality()) {
		bl.AddCmd("bst $in, ", in->GetQBit());
		bl.AddCmd("brtc ", block_id_str, "_bad_quality");
	}

	bl.AddCmd("bst $prev_in, ", prev_in->GetBit());
	bl.AddCmd("bld r24, ", in->GetBit());
	bl.AddCmd("bst $in, ", in->GetBit());
	bl.AddCmd("bld $prev_in, ", prev_in->GetBit());
	bl.AddCmd("brtc ", block_id_str, "_set_zero");
	
	bl.AddCmd("eor r24, $in");
	bl.AddCmd("bst r24, ", in->GetBit());
	bl.AddCmd("bld $out, ", out->GetBit());

	bl.AddCmd("rjmp ", block_id_str, "_set_good_quality");

	bl.AddLabel("_set_zero");
	bl.AddCmd("clt");
	bl.AddCmd("bld $out, ", out->GetBit());

	bl.AddLabel("_set_good_quality");
	bl.AddCmd("set");
	bl.AddLabel("_bad_quality");
	bl.AddCmd("bld $out,", out->GetQBit());

	m_code << bl;
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CNegativeEdge* block) {
	BLOCK_BEGIN(block);
	auto& slots = block->top->slots;

	const auto in = i_at(0).GetInputVariable();
	if (!in) return;

	AsmBlock bl(block_id_str);

	const auto prev_in = block->variable_.get();
	const auto out = o_at(0).GetVariable();

	gcc_->AddVariable(in);
	bl.AddOperand(OP_TYPE::GCC_OP_R, "$in", in);

	gcc_->AddVariable(out);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);

	gcc_->AddVariable(prev_in);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$prev_in", prev_in);

	bl.AddClobber(24);

	if (in->IsQuality()) {
		bl.AddCmd("bst $in, ", in->GetQBit());
		bl.AddCmd("brtc ", block_id_str, "_bad_quality");
	}

	bl.AddCmd("bst $prev_in, ", prev_in->GetBit());
	bl.AddCmd("bld r24, ", in->GetBit());
	bl.AddCmd("bst $in, ", in->GetBit());
	bl.AddCmd("bld $prev_in, ", prev_in->GetBit());
	bl.AddCmd("brts ", block_id_str, "_set_zero");

	bl.AddCmd("eor r24, $in");
	bl.AddCmd("bst r24, ", in->GetBit());
	bl.AddCmd("bld $out, ", out->GetBit());
	bl.AddCmd("rjmp ", block_id_str, "_set_good_quality");

	bl.AddLabel("_set_zero");
	bl.AddCmd("clt");
	bl.AddCmd("bld $out, ", out->GetBit());

	bl.AddLabel("_set_good_quality");
	bl.AddCmd("set");
	bl.AddLabel("_bad_quality");
	bl.AddCmd("bld $out, ", out->GetQBit());

	m_code << bl;
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CAnyEdge* block) {
	BLOCK_BEGIN(block);
	auto& slots = block->top->slots;

	const auto in = i_at(0).GetInputVariable();
	if (!in) return;

	AsmBlock bl(block_id_str);

	const auto prev_in = block->variable_.get();
	const auto out = o_at(0).GetVariable();

	gcc_->AddVariable(in);
	bl.AddOperand(OP_TYPE::GCC_OP_R, "$in", in);

	gcc_->AddVariable(out);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);

	gcc_->AddVariable(prev_in);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$prev_in", prev_in);

	bl.AddClobber(24);

	if (in->IsQuality()) {
		bl.AddCmd("bst $in, ", in->GetQBit());
		bl.AddCmd("brtc ", block_id_str, "_bad_quality");
	}

	bl.AddCmd("bst $prev_in, ", prev_in->GetBit());
	bl.AddCmd("bld r24, ", in->GetBit());
	bl.AddCmd("bst $in, ", in->GetBit());
	bl.AddCmd("bld $prev_in, ", prev_in->GetBit());
	bl.AddCmd("eor r24, $in");

	bl.AddCmd("bst r24, ", in->GetBit());
	bl.AddCmd("bld $out, ", out->GetBit());
	
	bl.AddCmd("set");
	bl.AddLabel("_bad_quality");
	bl.AddCmd("bld $out,", out->GetQBit());

	m_code << bl;
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CBinaryEncoder* block) {
	BLOCK_BEGIN(block);
	auto& slots = block->top->slots;

	if (!block->InputHasValidVariable()) return;

	AsmBlock bl(block_id_str);

	const auto out = o_at(0).GetVariable();

	gcc_->AddVariable(out);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);

	ForIn(slots) {
		const auto var = ins().GetInputVariable();
		if (var == nullptr) continue;
		gcc_->AddVariable(var);
		std::string op_name = "$in_" + std::to_string(ix + 1);
		bl.AddOperand(OP_TYPE::GCC_OP_R, op_name, var);

		int bit = ix % 8;

		if (var->IsQuality()) {
			if (var->IsBit()) {
				bl.AddCmd("bst ", op_name, ",", var->GetQBit());
				bl.AddCmd("brtc ", block_id_str, "_bad_q_end");
			} else {
				bl.AddCmd("tst ", op_name, "_q");
				bl.AddCmd("breq ", block_id_str, "_bad_q_end");
			}
		}

		bl.AddCmd("bst ", op_name, ",", var->GetBit());
		if (out->GetSize() == 1) {
			bl.AddCmd("bld $out,", bit);
		} else if (out->GetSize() == 2) {
			if (ix < 8) {
				bl.AddCmd("bld A$out, ", bit);
			} else {
				bl.AddCmd("bld B$out, ", bit);
			}
		} else {
			if (ix < 8) {
				bl.AddCmd("bld A$out, ", bit);
			} else if (ix >= 8 && ix < 16) {
				bl.AddCmd("bld B$out, ", bit);
			} else if (ix >= 16 && ix < 24) {
				bl.AddCmd("bld C$out, ", bit);
			} else {
				bl.AddCmd("bld D$out, ", bit);
			}
		}
	}
	bl.AddCmd("ldi $out_q, 0x01");
	bl.AddCmd("rjmp ", block_id_str, "_end");

	bl.AddLabel("_bad_q_end");
	bl.AddCmd("eor $out_q, $out_q");

	bl.AddLabel("_end");

	m_code << bl;
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CBinaryDecoder* block) {
	BLOCK_BEGIN(block);
	auto& slots = block->top->slots;

	AsmBlock bl(block_id_str);

	const auto in = i_at(0).GetInputVariable();
	if (!in) return;

	gcc_->AddVariable(in);
	bl.AddOperand(OP_TYPE::GCC_OP_R, "$in", in);

	auto begin = slots.begin_out(),
		end = slots.end_out();

	bl.AddClobber(16);
	bl.AddCmd("lds r16,", in->GetQAddr());

	int var_size = in->GetSize() * 8;

	std::string l(1, char('A' - 1));

	ForOutR(slots) {
		if (ix >= var_size) break;

		std::string b = "$out_" + std::to_string(ix);
		const auto v = ons().GetVariable();

		gcc_->AddVariable(v);
		bl.AddOperand(OP_TYPE::GCC_OP_RW, b, v);

		int bit = ix % 8;

		if (bit == 0) l = std::string(1, char(l[0] + 1));

		bl.AddCmd("bst r16, 0");
		bl.AddCmd("bld ", b, ", ", v->GetQBit());

		bl.AddCmd("bst ", l, "$in,", bit);
		bl.AddCmd("bld ", b, ", ", v->GetBit());
	}

	m_code << bl;
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CDelay* block) {
	BLOCK_BEGIN(block);

	auto& slots = block->top->slots;

	const auto in = i_at(0).GetInputVariable();
	if (!in) return;

	AsmBlock bl(block_id_str);

	const auto timer = block->variable_.get();
	const auto out = o_at(0).GetVariable();
	
	int cntAddr = info_.rmem.info + offsetof(net::RuntimeInfoController, u32_time);

	std::string cntAddr_a = std::to_string(cntAddr + 0);
	std::string cntAddr_b = std::to_string(cntAddr + 1);
	std::string cntAddr_c = std::to_string(cntAddr + 2);

	const auto& timeout = block->GetPropertyByKey<int>("timeout");
	DWORD dw = static_cast<DWORD>((float)timeout->GetValue() / 1000.0f / info_.mccfg.one_tick_time);

	std::string delay_a = std::to_string(dw & 0xFF);
	std::string delay_b = std::to_string((dw >> 8) & 0xFF);
	std::string delay_c = std::to_string((dw >> 16) & 0xFF);

	gcc_->AddVariable(in);
	gcc_->AddVariable(out);
	gcc_->AddVariable(timer);

	//	m_code << "uint32_t cnt;");
	//	m_code << "ATOMIC_BLOCK(ATOMIC_FORCEON) { cnt = dwCnt; }");
	//	bl.AddOperand(gcc__OP_R, "$cnt");

	bl.AddOperand(OP_TYPE::GCC_OP_R, "$in", in);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$mycnt", timer);

	bl.AddClobber(16);
	bl.AddClobber(17);
	bl.AddClobber(18);

	// Body
	if (in->IsQuality()) {
		bl.AddCmd("bst $in, ", in->GetQBit());
		bl.AddCmd("bld $out, ", out->GetQBit());
		bl.AddBranch("brtc", "_end");
	}

	bl.AddCmd("bst $in, ", in->GetBit());
	bl.AddCmd("brts ", block_id_str, "_do_stuff");
	bl.AddCmd("mov D$mycnt, r1");
	bl.AddCmd("rjmp ", block_id_str, "_set_value");

	// there is an input signal
	bl.AddLabel("_do_stuff");
	bl.AddCmd("cli");
	bl.AddCmd("lds r16,", cntAddr_a);
	bl.AddCmd("lds r17,", cntAddr_b);
	bl.AddCmd("lds r18,", cntAddr_c);
	bl.AddCmd("sei");
	// check previous state
	// we have already flag T set
	bl.AddCmd("tst D$mycnt");
	bl.AddCmd("breq ", block_id_str, "_load_value");
	bl.AddCmd("bst D$mycnt, 1");
	bl.AddCmd("brts ", block_id_str, "_set_value");
	bl.AddCmd("set");
	// test
	bl.AddCmd("sub r16, A$mycnt");
	bl.AddCmd("sbc r17, B$mycnt");
	bl.AddCmd("sbc r18, C$mycnt");
	bl.AddCmd("cpi r16, ", delay_a);
	bl.AddCmd("sbci r17, ", delay_b);
	bl.AddCmd("sbci r18, ", delay_c);
	bl.AddCmd("brcc ", block_id_str, "_set_value");
	bl.AddCmd("clt");
	bl.AddCmd("rjmp ", block_id_str, "_set_value");
	// previous input was inactiv
	bl.AddLabel("_load_value");
	bl.AddCmd("mov A$mycnt, r16");
	bl.AddCmd("mov B$mycnt, r17");
	bl.AddCmd("mov C$mycnt, r18");
	bl.AddCmd("bld D$mycnt, 0");
	bl.AddCmd("clt");
	//
	bl.AddLabel("_set_value");
	bl.AddCmd("bld $out, ", out->GetBit());
	bl.AddCmd("bld D$mycnt, 1");
	if (in->IsQuality()) {
		bl.AddLabel("_end");
	}
	m_code << bl;
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CCounter* block) {
	BLOCK_BEGIN(block);
	auto& slots = block->top->slots;

	const auto in = i_at(0).GetInputVariable();
	if (!in) return;

	const auto reset = i_at(1).GetInputVariable();

	AsmBlock bl(block_id_str);

	const auto prev_in = block->variable_.get();
	const auto out = o_at(0).GetVariable();

	gcc_->AddVariable(in);
	bl.AddOperand(OP_TYPE::GCC_OP_R, "$in", in);

	if (reset) {
		gcc_->AddVariable(reset);
		bl.AddOperand(OP_TYPE::GCC_OP_R, "$reset", reset);
	}

	gcc_->AddVariable(out);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);

	gcc_->AddVariable(prev_in);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$prev_in", prev_in);

	bl.AddClobber(24);

	if (reset) {
		if (reset->IsQuality()) {
			bl.AddCmd("bst $reset, ", reset->GetQBit());
			bl.AddCmd("brtc ", block_id_str, "_end");
		}

		bl.AddCmd("bst $reset, ", reset->GetBit());
		bl.AddCmd("brtc ", block_id_str, "_rst_not_set");
		
		bl.AddCmd("clr A$out");
		bl.AddCmd("clr B$out");
		bl.AddCmd("rjmp ", block_id_str, "_end");
		
		bl.AddLabel("_rst_not_set");
	}

	if (in->IsQuality()) {
		bl.AddCmd("bst $in, ", in->GetQBit());
		bl.AddCmd("brtc ", block_id_str, "_end");
	}

	bl.AddCmd("bst $prev_in, ", prev_in->GetBit());
	bl.AddCmd("bld r24, ", in->GetBit());
	bl.AddCmd("bst $in, ", in->GetBit());
	bl.AddCmd("bld $prev_in, ", prev_in->GetBit());
	bl.AddCmd("brtc ", block_id_str, "_end");
	
	bl.AddCmd("eor r24, $in");
	bl.AddCmd("andi r24, ", 1 << in->GetBit());
	bl.AddCmd("breq ", block_id_str, "_end");
	
	bl.AddCmd("sec");
	bl.AddCmd("adc A$out, r1");
	bl.AddCmd("adc B$out, r1");

	bl.AddLabel("_end");

	m_code << bl;
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CPulse *block) {
	BLOCK_BEGIN(block);

	auto& slots = block->top->slots;

	const auto in = i_at(0).GetInputVariable();
	if (!in) return;

	AsmBlock bl(block_id_str);

	const auto tmr_cur_cnt = block->variables_[CPulse::IV_TMR_CUR_CNT].get();
	const auto edge_prev_state = block->variables_[CPulse::IV_EDGE_PREVIOUS_STATE].get();

	const auto out = o_at(0).GetVariable();
	
	auto internal_counter_addr = info_.rmem.info + offsetof(net::RuntimeInfoController, u32_time);

	const auto fall = block->GetPropertyByKey<bool>("fall")->GetValue();
	const auto dw = static_cast<unsigned short>(
		static_cast<float>(block->GetPropertyByKey<unsigned short>("plsw")->GetValue()) / 1000.0f / info_.mccfg.one_tick_time);

	const auto delay_a = dw & 0xFF;
	const auto delay_b = (dw >> 8) & 0xFF;

	gcc_->AddVariable(in);
	gcc_->AddVariable(out);
	gcc_->AddVariable(edge_prev_state);
	gcc_->AddVariable(tmr_cur_cnt);

	bl.AddOperand(OP_TYPE::GCC_OP_R, "$in", in);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$edge_prev_state", edge_prev_state);
	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$tmr_cur_cnt", tmr_cur_cnt);
	
	bl.AddClobber(16);
	bl.AddClobber(17);

	if (in->IsQuality()) {
		bl.AddCmd("bst $in, ", in->GetQBit());
		bl.AddCmd("brtc ", block_id_str, "_set_quality");
	}

	bl.AddCmd("mov r16, $edge_prev_state");
	bl.AddCmd("bst $in, ", in->GetBit());
	bl.AddCmd("bld $edge_prev_state, ", edge_prev_state->GetBit());
	
	if (!fall) {
		bl.AddCmd("brtc ", block_id_str, "_edge_out");
		bl.AddCmd("com r16");
		bl.AddCmd("bst r16, ", edge_prev_state->GetBit());
		bl.AddCmd("brtc ", block_id_str, "_edge_out");
		bl.AddCmd("set");
		bl.AddLabel("_edge_out");
	} else {
		bl.AddCmd("brts ", block_id_str, "_edge_out_zero");
		bl.AddCmd("bst r16, ", edge_prev_state->GetBit());
		bl.AddCmd("brtc ", block_id_str, "_edge_out");
		bl.AddBranch("rjmp", "_edge_out");
		bl.AddLabel("_edge_out_zero");
		bl.AddCmd("clt");
		bl.AddLabel("_edge_out");
	}

	bl.AddCmd("cli");
	bl.AddCmd("lds r16, ", internal_counter_addr + 0);
	bl.AddCmd("lds r17, ", internal_counter_addr + 1);
	bl.AddCmd("sei");

	bl.AddCmd("brts ", block_id_str, "_reset_counter");

	bl.AddCmd("bst $out, ", out->GetBit());
	bl.AddCmd("brtc ", block_id_str, "_skip_set_out");

	bl.AddCmd("sub r16, A$tmr_cur_cnt");
	bl.AddCmd("sbc r17, B$tmr_cur_cnt");
	bl.AddCmd("cpi r16, ", delay_a);
	bl.AddCmd("sbci r17, ", delay_b);
	bl.AddCmd("brcs ", block_id_str, "_skip_set_out");
	
	bl.AddCmd("clt");	
	bl.AddCmd("rjmp ", block_id_str, "_set_out");

	bl.AddLabel("_reset_counter");
	bl.AddCmd("mov A$tmr_cur_cnt, r16");
	bl.AddCmd("mov B$tmr_cur_cnt, r17");
	bl.AddCmd("set");
	
	bl.AddLabel("_set_out");
	bl.AddCmd("bld $out, ", out->GetBit());
	bl.AddLabel("_skip_set_out");

	if (in->IsQuality()) {
		bl.AddCmd("set");
		bl.AddLabel("_set_quality");
		bl.AddCmd("bld $out, ", out->GetQBit());
	}
	
	m_code << bl;

	BLOCK_END(block);
}

// Math
void CAVR5CodeGenerator::Generate(CAdd* block) {
	BLOCK_BEGIN(block);
	auto& slots = block->top->slots;
	const auto in1 = i_at(0).GetInputVariable();
	const auto in2 = i_at(1).GetInputVariable();
	const auto out = o_at(0).GetVariable();
	if (!in1 && !in2) {
	} else if (!in1 && in2) {
		gcc_->AddVariable(in2);
		gcc_->AddVariable(out);
		m_code << out->to_vardecl() << " = " <<
			(in2->IsBit() ? CastBit(in2) : in2->to_vardecl()) << ";\n";
		m_code << out->to_vardeclq() << " = " <<
			(in2->IsBit() ? CastQBit(in2) : in2->to_vardeclq()) + ";\n";
	} else if (in1 && !in2) {
		gcc_->AddVariable(in1);
		gcc_->AddVariable(out);
		m_code << out->to_vardecl() << " = " <<
			(in1->IsBit() ? CastBit(in1) : in1->to_vardecl()) << ";\n";
		m_code << out->to_vardeclq() << " = " <<
			(in1->IsBit() ? CastQBit(in1) : in1->to_vardeclq()) << ";\n";
	} else {
		int t1 = in1->GetType(), t2 = in2->GetType();
		gcc_->AddVariable(in1);
		gcc_->AddVariable(in2);
		gcc_->AddVariable(out);
		if ((t1 & npsys::nptype::FLOAT_VALUE) || (t2 & npsys::nptype::FLOAT_VALUE)) {
			AddLibrary("avr_libm");
		}
		m_code << out->to_vardecl() << " = " <<
			(in1->IsBit() ? CastBit(in1) : in1->to_vardecl()) <<
			" + " << (in2->IsBit() ? CastBit(in2) : in2->to_vardecl()) << ";\n"
		<< out->to_vardeclq() << " = " <<
			(in1->IsBit() ? CastQBit(in1) : in1->to_vardeclq()) <<
			" & " <<
			(in2->IsBit() ? CastQBit(in2) : in2->to_vardeclq())
			+ ";\n";
	}
	BLOCK_END(block);
}

/*
void CAVR5CodeGenerator::Generate(CSub* pSub) {
	BLOCK_BEGIN(pSub);
	auto it = pSub->begin();
	const auto in1 = static_cast<CInputSlot*>((*it++))->var.get()();
	const auto in2 = static_cast<CInputSlot*>((*it++))->var.get()();
	const auto out = (*it)->var.get();
	if (in1 && in2) {
		int t1 = in1->GetType(),
			t2 = in2->GetType();
		gcc_->AddVariable(in1);
		gcc_->AddVariable(in2);
		gcc_->AddVariable(out);
		//
		if ((t1 & Variable::NPT_F32) || (t2 & Variable::NPT_F32)) {
			AddLibrary("avr_libm");
		}
		m_code << out->to_vardecl() + " = " + in1->to_vardecl() + " - " + in2->to_vardecl() + ";");
	}
}

void CAVR5CodeGenerator::Generate(CMul* pMul) {
	BLOCK_BEGIN(pMul);
	auto it = pMul->begin();
	const auto in1 = static_cast<CInputSlot*>((*it++))->var.get()();
	const auto in2 = static_cast<CInputSlot*>((*it++))->var.get()();
	const auto out = (*it)->var.get();
	if (in1 && in2) {
		int t1 = in1->GetType(),
			t2 = in2->GetType();

		gcc_->AddVariable(in1);
		gcc_->AddVariable(in2);
		gcc_->AddVariable(out);
		//
		if ((t1 & Variable::NPT_F32) || (t2 & Variable::NPT_F32)) {
			AddLibrary("avr_libm");
		}
		m_code << out->to_vardecl() + " = " + in1->to_vardecl() + " * " + in2->to_vardecl() + ";");
	}
}
void CAVR5CodeGenerator::Generate(CDiv* pDiv) {
	BLOCK_BEGIN(pDiv);
	auto it = pDiv->begin();
	const auto in1 = static_cast<CInputSlot*>((*it++))->var.get()();
	const auto in2 = static_cast<CInputSlot*>((*it++))->var.get()();
	const auto out = (*it)->var.get();
	if (in1 && in2) {
		int t1 = in1->GetType();
		int	t2 = in2->GetType();

		gcc_->AddVariable(in1);
		gcc_->AddVariable(in2);
		gcc_->AddVariable(out);
		//
	//	m_AdditionalLib.emplace("libgcc_");
		if ((t1 & Variable::NPT_F32) || (t2 & Variable::NPT_F32)) {
			AddLibrary("avr_libm");
		}
		m_code << out->to_vardecl() + " = " + in1->to_vardecl() + " / " + in2->to_vardecl() + ";");
	}
}
void CAVR5CodeGenerator::Generate(CPID* pid) {
	BLOCK_BEGIN(pid);

	auto begin_i = pid->slots.begin_in();
	auto begin_o = pid->slots.begin_out();

	const auto in1 = static_cast<CInputSlot*>(*begin_i)->var.get()();
	const auto in2 = static_cast<CInputSlot*>(*std::next(begin_i))->var.get()();
	const auto out = static_cast<COutputSlot*>(*begin_o)->var.get();

	if (in1 && in2) {
		int t1 = in1->GetType(),
			t2 = in2->GetType();
		gcc_->AddVariable(in1);
		gcc_->AddVariable(in2);
		gcc_->AddVariable(out);
		AddLibrary("avr_libm");
		AddLibrary("pid");
		m_code << "pid_run((void*)0x" + to_hex(pid->variables_[0]->GetAddr()) + "," + in1->to_vardecl() + "," + in2->to_vardecl() + ");");
	}
}
*/

void CAVR5CodeGenerator::Generate(CComparator* block) {
	BLOCK_BEGIN(block);

	auto& slots = block->top->slots;
	const auto in1 = i_at(0).GetInputVariable();
	const auto in2 = i_at(1).GetInputVariable();
	
	if (!in1 || !in2) return;
	
	if (in1->IsBit() || in2->IsBit()) {
		std::cout << "Block: \"" << block->GetName() 
			<< "\" will not be loaded: discrete inputs are not supported" << std::endl;
	}

	gcc_->AddVariable(in1);
	gcc_->AddVariable(in2);
	
	if (in1->GetType() & npsys::nptype::FLOAT_VALUE ||
		in2->GetType() & npsys::nptype::FLOAT_VALUE) AddLibrary("avr_libm");

	m_code << "uint8_t q_val;\n";
	
	if (in1->IsQuality() || in2->IsQuality()) {
		if (in1->IsQuality() && in2->IsQuality()) {
			m_code << "if (" << in1->to_vardeclq() << " == 0x01 && " << in2->to_vardeclq() << " == 0x01) q_val = 0x01;\n";
		} else if (in1->IsQuality()) {
			m_code << "if (" << in1->to_vardeclq() << " == 0x01) q_val = 0x01;\n";
		} else {
			m_code << "if (" << in2->to_vardeclq() << " == 0x01) q_val = 0x01;\n";
		}
		m_code << "else q_val = 0x00;\n";
	} else {
		m_code << "q_val = 0x01\n";
	}
	

	if (auto slot = slots.GetSlotByName("EQ"); slot) {
		AsmBlock bl(block_id_str);
		const auto out = slot->GetVariable();
		
		gcc_->AddVariable(out);
		bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);
		bl.AddOperand(OP_TYPE::GCC_OP_R, "$q_val", nullptr);
		m_code << "if (" << in1->to_vardecl() << " == " << in2->to_vardecl() << ") {\n";
		m_code << "__asm__ __volatile__ (\"set\");\n";
		m_code << "} else {\n";
		m_code << "\t__asm__ __volatile__ (\"clt\");\n";
		m_code << "}\n";
		bl.AddCmd("bld $out, ", out->GetBit());
		bl.AddCmd("bst $q_val, 0");
		bl.AddCmd("bld $out, ", out->GetQBit());

		m_code << bl;
	}

	if (auto slot = slots.GetSlotByName("NE"); slot) {
		AsmBlock bl(block_id_str);
		const auto out = slot->GetVariable();
		
		gcc_->AddVariable(out);
		bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);
		bl.AddOperand(OP_TYPE::GCC_OP_R, "$q_val", nullptr);
		m_code << "if (" << in1->to_vardecl() << " != " << in2->to_vardecl() << ") {\n";
		m_code << "__asm__ __volatile__ (\"set\");\n";
		m_code << "} else {\n";
		m_code << "\t__asm__ __volatile__ (\"clt\");\n";
		m_code << "}\n";
		bl.AddCmd("bld $out, ", out->GetBit());
		bl.AddCmd("bst $q_val, 0");
		bl.AddCmd("bld $out, ", out->GetQBit());
	
		m_code << bl;
	}

	if (auto slot = slots.GetSlotByName("GT"); slot) {
		AsmBlock bl(block_id_str);
		const auto out = slot->GetVariable();
		
		gcc_->AddVariable(out);
		bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);
		bl.AddOperand(OP_TYPE::GCC_OP_R, "$q_val", nullptr);
		m_code << "if (" << in1->to_vardecl() << " > " << in2->to_vardecl() << ") {\n";
		m_code << "__asm__ __volatile__ (\"set\");\n";
		m_code << "} else {\n";
		m_code << "\t__asm__ __volatile__ (\"clt\");\n";
		m_code << "}\n";
		bl.AddCmd("bld $out, ", out->GetBit());
		bl.AddCmd("bst $q_val, 0");
		bl.AddCmd("bld $out, ", out->GetQBit());
	
		m_code << bl;
	}

	if (auto slot = slots.GetSlotByName("GE"); slot) {
		AsmBlock bl(block_id_str);
		const auto out = slot->GetVariable();
		
		gcc_->AddVariable(out);
		bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);
		bl.AddOperand(OP_TYPE::GCC_OP_R, "$q_val", nullptr);
		m_code << "if (" << in1->to_vardecl() << " >= " << in2->to_vardecl() << ") {\n";
		m_code << "__asm__ __volatile__ (\"set\");\n";
		m_code << "} else {\n";
		m_code << "\t__asm__ __volatile__ (\"clt\");\n";
		m_code << "}\n";
		bl.AddCmd("bld $out, ", out->GetBit());
		bl.AddCmd("bst $q_val, 0");
		bl.AddCmd("bld $out, ", out->GetQBit());

		m_code << bl;
	}

	if (auto slot = slots.GetSlotByName("LT"); slot) {
		AsmBlock bl(block_id_str);
		const auto out = slot->GetVariable();
		
		gcc_->AddVariable(out);
		bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);
		bl.AddOperand(OP_TYPE::GCC_OP_R, "$q_val", nullptr);
		m_code << "if (" << in1->to_vardecl() << " < " << in2->to_vardecl() << ") {\n";
		m_code << "__asm__ __volatile__ (\"set\");\n";
		m_code << "} else {\n";
		m_code << "\t__asm__ __volatile__ (\"clt\");\n";
		m_code << "}\n";
		bl.AddCmd("bld $out, ", out->GetBit());
		bl.AddCmd("bst $q_val, 0");
		bl.AddCmd("bld $out, ", out->GetQBit());

		m_code << bl;
	}

	if (auto slot = slots.GetSlotByName("LE"); slot) {
		AsmBlock bl(block_id_str);
		const auto out = slot->GetVariable();
		
		gcc_->AddVariable(out);
		bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);
		bl.AddOperand(OP_TYPE::GCC_OP_R, "$q_val", nullptr);
		m_code << "if (" << in1->to_vardecl() << " <= " << in2->to_vardecl() << ") {\n";
		m_code << "__asm__ __volatile__ (\"set\");\n";
		m_code << "} else {\n";
		m_code << "\t__asm__ __volatile__ (\"clt\");\n";
		m_code << "}\n";
		bl.AddCmd("bld $out, ", out->GetBit());
		bl.AddCmd("bst $q_val, 0");
		bl.AddCmd("bld $out, ", out->GetQBit());

		m_code << bl;
	}
	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CBlockFunction* block) {
	BLOCK_BEGIN(block);
	auto& slots = block->top->slots;
	const auto in = i_at(0).GetInputVariable();
	const auto out = o_at(0).GetVariable();

	if (!in) return;

	gcc_->AddVariable(in);
	gcc_->AddVariable(out);
	
	auto in_var = in->to_vardecl();
	auto in_varq = in->to_vardeclq();

	AddLibrary("avr_libm");

	static const auto pattern = std::regex("IN");
  auto exp = std::regex_replace(block->equation_, pattern, in_var);
	m_code << out->to_vardecl() << " = " << exp << ";\n";
	
	BLOCK_END(block);
}


void CAVR5CodeGenerator::Generate(CAlarmHigh* block) {
	GenerateAlarmBlock(block, true);
}

void CAVR5CodeGenerator::Generate(CAlarmLow* block) {
	GenerateAlarmBlock(block, false);
}

void CAVR5CodeGenerator::GenerateAlarmBlock(CBlock* block, bool high) {
	BLOCK_BEGIN(block);

	auto& slots = block->top->slots;
	const auto value = i_at(0).GetInputVariable();
	const auto sp = i_at(1).GetInputVariable();
	const auto out = o_at(0).GetVariable();

	if (!value || !sp) return;

	if (value->GetType() & npsys::nptype::BIT_VALUE || sp->GetType() & npsys::nptype::BIT_VALUE) {
		std::cerr << block->GetName() << ": discretre inputs are not supported\n";
		return;
	}

	const auto& hyst = block->GetPropertyByKey<float>("hyst");

	std::string val;
	if (value->GetType() & npsys::nptype::FLOAT_VALUE || sp->GetType() & npsys::nptype::FLOAT_VALUE) {
		AddLibrary("avr_libm");
		val = std::to_string(hyst->GetValue());
	} else {
		val = std::to_string((int)hyst->GetValue());
	}

	gcc_->AddVariable(value);
	gcc_->AddVariable(sp);
	gcc_->AddVariable(out);

	if (value->IsQuality() || sp->IsQuality()) {
		m_code << "unsigned char	quality = 0x01;\n";
	}
	
	m_code << "unsigned char value = 0xFF;\n";

	if (value->IsQuality()) {
		m_code << "if (" << value->to_vardeclq() << " == 0) {\n"
			"\tquality = 0; }\n";
	}

	if (value->IsQuality()) {
		m_code << "if (" << sp->to_vardeclq() << " == 0) {\n"
			"\tquality = 0; }\n";
	}

	if (high) {
		m_code <<
		"if (" << value->to_vardecl() << " >= " << sp->to_vardecl() << ") {\n"
			"\tvalue = 0x01; }\n"
		"else if (" << value->to_vardecl() << " < " << sp->to_vardecl() << " - " << val << ") {\n"
			"\tvalue = 0x00; }\n";
	} else {
		m_code <<
		"if (" << value->to_vardecl() << " < " << sp->to_vardecl() << ") {\n"
			"\tvalue = 0x01; }\n"
		"else if (" << value->to_vardecl() << " - " << val << " > " << sp->to_vardecl() << ") {\n"
			"\tvalue = 0x00; }\n";
	}

	AsmBlock bl(block_id_str);

	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);
	bl.AddOperand(OP_TYPE::GCC_OP_R, "$value", nullptr);
	bl.AddOperand(OP_TYPE::GCC_OP_R, "$quality", nullptr);
	
	bl.AddCmd("bst $quality, 0");
	bl.AddCmd("bld $out, ", out->GetQBit());

	bl.AddCmd("cpi $value, 0xFF");
	bl.AddCmd("breq .+4");
	bl.AddCmd("bst $value, 0");
	bl.AddCmd("bld $out, ", out->GetBit());

	m_code << bl;

	BLOCK_END(block);
}

void CAVR5CodeGenerator::Generate(CBlockSchedule* block) {
	BLOCK_BEGIN(block);

	auto tm_size = block->variables_.size() / 2;

	if (tm_size == 0) return;

	auto& slots = block->top->slots;
	const auto enabled = i_at(0).GetInputVariable();
	const auto out = o_at(0).GetVariable();
	const auto hour = o_at(1).GetVariable();
	const auto min = o_at(2).GetVariable();
	const auto sec = o_at(3).GetVariable();

	gcc_->AddVariable(out);
	gcc_->AddVariable(hour);
	gcc_->AddVariable(min);
	gcc_->AddVariable(sec);

	AsmBlock bl(block_id_str);

	bl.AddOperand(OP_TYPE::GCC_OP_RW, "$out", out);
	bl.AddOperand(OP_TYPE::GCC_OP_R, "$hour", hour);
	bl.AddOperand(OP_TYPE::GCC_OP_R, "$min", min);
	bl.AddOperand(OP_TYPE::GCC_OP_R, "$sec", sec);
	bl.AddClobber(18); // counter
	bl.AddClobber(19); // temp
	bl.AddClobber(30); // zl
	bl.AddClobber(31); // zh
	
	bl.AddCmd("clt");
	bl.AddCmd("tst $hour_q");
	bl.AddBranch("breq", "_bad_quality");

	if (enabled) {
		gcc_->AddVariable(enabled);
		bl.AddOperand(OP_TYPE::GCC_OP_R, "$enabled", enabled);

		if (enabled->IsQuality()) {
			bl.AddCmd("bst $enabled, ", enabled->GetQBit());
			bl.AddBranch("brtc", "_bad_quality");
		}

		bl.AddCmd("bst $enabled, ", enabled->GetBit());
		bl.AddBranch("brtc", "_copy_t_bit_good_quality");
		bl.AddCmd("clt");
	}

	bl.AddCmd("cpi $sec, ", block->GetPropertyByKey<int>("active_time")->GetValue());
	bl.AddBranch("brsh", "_copy_t_bit_good_quality");

	bl.AddBranch("rjmp", "_loop_setup");

	// bad quality
	bl.AddLabel("_bad_quality");
	bl.AddCmd("bld $out, ", out->GetQBit());
	bl.AddBranch("rjmp", "_block_exit");
	
	// loop setup
	bl.AddLabel("_loop_setup");
	bl.AddCmd("ldi r18, ", tm_size);
	bl.AddCmd("ldi zl, ", block->GetOrigin() & 0xff);
	bl.AddCmd("ldi zh, ", (block->GetOrigin() >> 8) & 0xff);
	// loop
	bl.AddLabel("_loop");
	bl.AddCmd("LDD r19, z + 0");
	bl.AddCmd("cpse r19, $hour");
	bl.AddBranch("rjmp", "_loop_continue");

	bl.AddCmd("LDD r19, z + 1");
	bl.AddCmd("cpse r19, $min");
	bl.AddBranch("rjmp", "_loop_continue");

	bl.AddCmd("set");
	bl.AddBranch("rjmp", "_copy_t_bit_good_quality");

	bl.AddLabel("_loop_continue");
	bl.AddCmd("adiw z, 2");
	bl.AddCmd("dec r18");
	bl.AddBranch("brne", "_loop");
	// loop end
	bl.AddLabel("_copy_t_bit_good_quality");
	
	bl.AddCmd("bld $out, ", out->GetBit());
	bl.AddCmd("set");
	bl.AddCmd("bld $out, ", out->GetQBit());

	bl.AddLabel("_block_exit");

	m_code << bl;

	BLOCK_END(block);
}


void CAVR5CodeGenerator::Generate(CSub*) {}
void CAVR5CodeGenerator::Generate(CMul*) {}
void CAVR5CodeGenerator::Generate(CDiv*) {}
void CAVR5CodeGenerator::Generate(CTime*) {}
void CAVR5CodeGenerator::Generate(CPID*) {}

