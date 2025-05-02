// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "opcodes.h"
#include "cpu.h"
#include "atmega16_def.h"
#include <atomic>
#include <avr_firmware/net.h>
#include <avr_firmware/net_dbg.h>
#include "intrisics.h"


#define BIT(n) (1 << n)

#define Register8(n) r_file_[n]
#define IO_Register8(n) io_space_[n]

#define Register16(n) (*reinterpret_cast<uint16_t*>(r_file_ + n))
#define IO_Register16(n) (*reinterpret_cast<uint16_t*>(io_space_ + n))

#define SREG (*sreg_ptr_)
#define STACK (*sp_ptr_)

#define PC pc_
#define CC cycle_count_

#define zl 30
#define zh 31

#define IndexRegX (*reinterpret_cast<uint16_t*>(&sram_[0] + 26))
#define IndexRegY (*reinterpret_cast<uint16_t*>(&sram_[0] + 28))
#define IndexRegZ (*reinterpret_cast<uint16_t*>(&sram_[0] + 30))

#define R0R1_PTR ((void*)r_file_)

// SREG
#define I	7
#define T	6
#define H	5
#define S	4
#define V	3
#define N	2
#define Z	1
#define C	0

#define GETF_CARRY() (SREG & BIT(C))
#define GETF_ZERO() (SREG & BIT(Z))

/*
0	CF	Carry Flag: Set by arithmetic instructions which generate either a carry or borrow. Set when an operation generates a carry to or a borrow from a destination operand.
2	PF	Parity flag: Set by most CPU instructions if the least significant (aka the low-order bits) of the destination operand contain an even number of 1's.
4	AF	Auxiliary Carry Flag: Set if there is a carry or borrow involving bit 4 of EAX. Set when a CPU instruction generates a carry to or a borrow from the low-order 4 bits of an operand. This flag is used for binary coded decimal (BCD) arithmetic.
6	ZF	Zero Flag: Set by most instructions if the result an operation is binary zero.
7	SF	Sign Flag: Most operations set this bit the same as the most significant bit (aka high-order bit) of the result. 0 is positive, 1 is negative.
8	TF	Trap Flag: (sometimes named a Trace Flag.) Permits single stepping of programs. After executing a single instruction, the processor generates an internal exception 1. When Trap Flag is set by a program, the processor generates a single-step interrupt after each instruction. A debugging program can use this feature to execute a program one instruction at a time.
9	IF	Interrupt Enable Flag: when set, the processor recognizes external interrupts on the INTR pin. When set, interrupts are recognized and acted on as they are received. The bit can be cleared to turn off interrupt processing temporarily.
10	DF	Direction Flag: Set and cleared using the STD and CLD instructions. It is used in string processing. When set to 1, string operations process down from high addresses to low addresses. If cleared, string operations process up from low addresses to high addresses.
11	OF	Overflow Flag: Most arithmetic instructions set this bit, indicating that the result was too large to fit in the destination. When set, it indicates that the result of an operation is too large or too small to fit in the destination operand.
12-13	IOPL	Input/Output privilege level flags: Used in protected mode to generate four levels of security.
14	NT	Nested Task Flag: Used in protected mode. When set, it indicates that one system task has invoked another via a CALL Instruction, rather than a JMP.
16	RF	Resume Flag: Used by the debug Register8s DR6 and DR7. It enables you to turn off certain exceptions while debugging code.
17	VM	Virtual 8086 Mode flag: Permits 80386 to behave like a high speed 8086.
*/

// EFLAGS
#define CF 0
#define AF 4
#define ZF 6
#define SF 7
#define OF 11

SIM_IMPORT_EXPORT AVRCore::AVRCore(MICROCONTROLLER mc, Flash& flash, sram_t& sram, eeprom_t& eeprom,
	uint16_t* sp_ptr, uint8_t* sreg_ptr, uint8_t* ucsra_ptr, int page_size, uint8_t dev_addr)
	: mc_(mc)
	, flash_(flash)
	, sram_(sram)
	, eeprom_(eeprom)
	, sreg_ptr_(sreg_ptr)
	, ucsra_ptr_(ucsra_ptr)
	, sp_ptr_(sp_ptr)
	, dev_addr_(dev_addr)
	, page_size_(page_size)
{
	r_file_ = &sram_[0];
	io_space_ = &sram_[0] + 0x20;
	pc_ = 0;
	cycle_count_ = 1;
	last_pc_.resize(16);

	spm_buf_.reset(new uint16_t[page_size / 2]);
	memset(spm_buf_.get(), 0xFF, page_size);
}

inline int AVRCore::DecodeInstruction(uint16_t instruction) const {
	int i = 0;
	while (((instruction & masks[i]) != patterns[i]) && (++i != OP_UNKNOWN));
	return i;
}
SIM_IMPORT_EXPORT void AVRCore::Decode(uint16_t begin_w, uint16_t end_w) {
	for (uint16_t i = begin_w; i < end_w; ++i) {
		auto& instruction = flash_.at(i).instruction;
		int index = DecodeInstruction(instruction);
		auto opcode = instructions[index].opcode;
		assert(opcode == index);
		auto size = instructions[index].size;
		if (opcode == OP_OUT) {
			auto r  = (instruction >> 4) & 0x1F;
			auto addr = (instruction & 0x0F) | ((instruction & 0x600) >> 5);
			if (addr == 0x0C) { // uart udr
				instruction = 0x0020 | r;
				opcode = OP_FAKE_UDR_OUT;
			}
		}
		flash_[i].op = opcode;
		flash_[i].size = size;
	}
}

#ifdef _MSC_VER
SIM_IMPORT_EXPORT int32_t AVRCore::Draw(HDC hdc) const {
	std::stringstream ss;

	ss << std::hex << std::setfill('0');

	static const char c_reg_file[] = "REGISTER_FILE:";

	int32_t y = 0, ch;

	auto draw_line = [&]() {
		std::string str(ss.str());
		ss.str(std::string());
		::TextOutA(hdc, 0, y, str.c_str(), static_cast<int>(str.length()));
		y += ch;
	};

	{
		RECT r;
		::DrawTextA(hdc, c_reg_file, sizeof(c_reg_file), &r, DT_CALCRECT);
		ch = r.bottom - r.top;
	}

	ss << "REGISTER_FILE:";
	draw_line();

	ss << "SREG:"
		" I:" << (SREG & BIT(I) ? 1 : 0) <<
		" T:" << (SREG & BIT(T) ? 1 : 0) <<
		" H:" << (SREG & BIT(H) ? 1 : 0) <<
		" S:" << (SREG & BIT(S) ? 1 : 0) <<
		" V:" << (SREG & BIT(V) ? 1 : 0) <<
		" N:" << (SREG & BIT(N) ? 1 : 0) <<
		" Z:" << (SREG & BIT(Z) ? 1 : 0) <<
		" C:" << (SREG & BIT(C) ? 1 : 0);
	draw_line();

	ss << "PC: 0x" << std::setw(4) << int(PC * 2) << " SP: 0x" << std::setw(4) << int(STACK);
	draw_line();

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 4; j++) {
			ss << "R" << std::dec << std::setw(2) << int(8 * j + i) <<
				" 0x" << std::hex << std::setw(2) << int(Register8(8 * j + i)) << "  ";
		}
		draw_line();
	}

	ss << "SRAM:";
	draw_line();

	for (int i = 0x60, k = 1; i < 0x460; i++, k++) {
		if (k == 16) {
			k = 0;
			ss << std::setw(2) << int(Register8(i));
			draw_line();
		} else if (k % 4 == 0) {
			ss << std::setw(2) << int(Register8(i)) << " | ";
		} else {
			if (k == 1) {
				ss << std::setw(4) << i << "   " << std::setw(2) << int(Register8(i)) << " ";
			} else {
				ss << std::setw(2) << int(Register8(i)) << " ";
			}
		}
	}

	ss << "FLASH:";
	draw_line();

	size_t flash_size = flash_.size();

	for (size_t i = 0, k = 1; i < flash_size; i++, k++) {
		if (k == 8) {
			k = 0;
			ss << std::setw(4) << int(flash_[i].instruction);
			draw_line();
		} else if (k % 4 == 0) {
			ss << std::setw(4) << int(flash_[i].instruction) << " | ";
		} else {
			if (k == 1) {
				ss << std::setw(4) << i * 2 << "   " << std::setw(4) << int(flash_[i].instruction) << " ";
			} else {
				ss << std::setw(4) << int(flash_[i].instruction) << " ";
			}
		}
	}

	return y;
}
#endif // _MSC_VER


SIM_IMPORT_EXPORT void AVRCore::PrintRegisterFile() const {
	std::cout << "SREG:"
		"\tI:" << (SREG & BIT(I) ? 1 : 0) <<
		"\tT:" << (SREG & BIT(T) ? 1 : 0) <<
		"\tH:" << (SREG & BIT(H) ? 1 : 0) <<
		"\tS:" << (SREG & BIT(S) ? 1 : 0) <<
		"\tV:" << (SREG & BIT(V) ? 1 : 0) <<
		"\tN:" << (SREG & BIT(N) ? 1 : 0) <<
		"\tZ:" << (SREG & BIT(Z) ? 1 : 0) <<
		"\tC:" << (SREG & BIT(C) ? 1 : 0) <<
		std::endl;

	printf("PC: 0x%.4x\t", PC * 2);
	printf("SP: 0x%.4X\t", STACK);
	printf("CC: %d\n", CC);

	//	printf("Register File:\n");
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 4; j++) {
			printf("R%.2d 0x%.2x   ", 8 * j + i, Register8(8 * j + i));
		}
		std::cout << "\n";
	}
}

SIM_IMPORT_EXPORT void AVRCore::PrintSRAM() const {
	//	printf("Data Memory:\n");
	for (int i = 0x60, k = 1; i < 0x460; i++, k++) {
		if (k == 16) {
			printf("0x%.2x\n", Register8(i));
			k = 0;
		} else if (k % 4 == 0) {
			printf("0x%.2x | ", Register8(i));
		} else {
			if (k == 1) {
				printf("0x%.4x   0x%.2x ", i, Register8(i));
			} else {
				printf("0x%.2x ", Register8(i));
			}
		}
	}
}

SIM_IMPORT_EXPORT void AVRCore::PrintCurrentInstruction() const {
	std::cout << instructions[flash_[PC].op].mnemonic << "\n";
}

uint32_t AVRCore::do_spm() {
	int page_num;
	int word_num;

	uint16_t* buf = spm_buf_.get();
	uint16_t r0r1 = *static_cast<uint16_t*>(R0R1_PTR);
	uint16_t z = IndexRegZ;

	if (mc_ == MC_ATMEGA8) {
		page_num = (z >> 6) & 0x7F;
		word_num = (z >> 1) & 0x1F;
	} else if (mc_ == MC_ATMEGA16) {
		page_num = (z >> 7) & 0x7F;
		word_num = (z >> 1) & 0x3F;
	} else {
		assert(false);
	}

	uint8_t& SPMCR = sram_[Atmega16::SPMCR];
	size_t page_begin_w = page_num * page_size_ / 2;
	size_t page_end_w = (page_num + 1) * page_size_ / 2;

	if (SPMCR & (1 << Atmega16::SPMEN)) {
		if (SPMCR & (1 << Atmega16::PGERS)) {
			for (size_t i = page_begin_w; i < page_end_w; ++i) {
				flash_[i].instruction = 0xFFFF;
				flash_[i].op = AVR_OPCODE::OP_UNKNOWN;
				flash_[i].size = 0;
			}
			memset(buf, 0xFF, page_size_);
		} else if (SPMCR & (1 << Atmega16::PGWRT)) {
			for (size_t i = page_begin_w, j = 0; i < page_end_w; ++i, ++j) {
				flash_[i].instruction = buf[j];
			}
			Decode(static_cast<uint16_t>(page_begin_w), static_cast<uint16_t>(page_end_w));
		} else if (SPMCR & (1 << Atmega16::RWWSRE)) {
			// nothing to do here
		} else {
			buf[word_num] = r0r1;
		}
		SPMCR &= ~(1 << Atmega16::SPMEN);
	}
	return 4;
}

uint32_t AVRCore::write_eeprom() {
	uint8_t& eecr = sram_[Atmega16::EECR];
	uint16_t addr = (sram_[Atmega16::EEARH] << 8) | sram_[Atmega16::EEARL];
	uint8_t eedr = sram_[Atmega16::EEDR];
	eeprom_[addr] = eedr;
	eecr &= ~(1 << Atmega16::EEWE);
	return 2;
}

uint32_t AVRCore::read_eeprom() {
	uint8_t& eecr = sram_[Atmega16::EECR];
	uint16_t addr = (sram_[Atmega16::EEARH] << 8) | sram_[Atmega16::EEARL];
	uint8_t& eedr = sram_[Atmega16::EEDR];
	eedr = eeprom_[addr];
	eecr &= ~(1 << Atmega16::EERE);
	return 4;
}

inline void AVRCore::normalize_pc() {
	int fsize = (int)flash_size();
	if (PC >= fsize) {
		PC -= fsize;
	} else if (PC < 0) {
		PC += fsize;
	}
}

// TODO: add sram memory boundary check
SIM_IMPORT_EXPORT int AVRCore::Step() {
	if (ev_list_.increase_tcnt0) {
		sram_[0x52]++;
		ev_list_.increase_tcnt0 = 0;
		ev_list_.tcnt0_increased = true;
	}
	if (ev_list_.increase_tcnt2) {
		sram_[0x44]++;
		ev_list_.increase_tcnt2 = 0;
		ev_list_.tcnt2_increased = true;
	}
	last_pc_.push_back(PC);
	if ((SREG & (1 << I)) && !interrupts_.empty()) {
		uint16_t int_addr;
		bool consumed = interrupts_.consume_one(
			[&int_addr](uint16_t addr) {
			int_addr = addr;
		});
		if (consumed) {
			sram_.at(STACK) = PC & 0xFF;
			sram_.at(STACK - 1) = (PC >> 8) & 0xFF;
			STACK = STACK - 2;
			PC = int_addr;
			SREG &= ~(1 << I);
			CC = CC + 4;
			return 4;
		}
	}

	uint8_t reg_n, bit_n, tmp8, Rd, Rr, K, A, q, b;
	uint16_t tmp16;
	int16_t offset;
	uint32_t cc = CC;
	uint64_t eflags;

	uint8_t& eecr = sram_[Atmega16::EECR];

	if (eecr & (1 << Atmega16::EERE)) {
		CC = CC + read_eeprom();
		return CC - cc;
	}
	if (eecr & (1 << Atmega16::EEWE)) {
		CC = CC + write_eeprom();
		return CC - cc;
	}

#ifdef _MSC_VER
	union {
		uint8_t u8;
		uint8_t u16;
		uint8_t u32;
    } nu;
#endif

	uint8_t h, s, v, n, z, _z, c;

	auto& cmd = flash_.at(PC);

	uint16_t instruction = cmd.instruction;
	AVR_OPCODE op = static_cast<AVR_OPCODE>(cmd.op);

	switch (op) {
	case OP_NOP:
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_CLC:
		SREG &= ~(1 << C);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_CLH:
		SREG &= ~(1 << H);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_CLI:
		SREG &= ~(1 << I);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_CLN:
		SREG &= ~(1 << N);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_CLS:
		SREG &= ~(1 << S);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_CLT:
		SREG &= ~(1 << T);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_CLV:
		SREG &= ~(1 << V);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_CLZ:
		SREG &= ~(1 << Z);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SEC:
		SREG |= (1 << C);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SEH:
		SREG |= (1 << H);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SEI:
		SREG |= (1 << I);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SEN:
		SREG |= (1 << N);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SES:
		SREG |= (1 << S);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SET:
		SREG |= (1 << T);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SEV:
		SREG |= (1 << V);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SEZ:
		SREG |= (1 << Z);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_LDI:
		reg_n = ((instruction >> 4) & 0x0F) + 16;
		Register8(reg_n) = (instruction & 0x0F) | ((instruction >> 4) & 0xF0);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_COM:
		reg_n = (instruction >> 4) & 0x1F;
		Register8(reg_n) = ~Register8(reg_n);

		n = Register8(reg_n) & (1 << 7) ? 1 : 0;
		z = Register8(reg_n) ? 0 : 1;
		c = 1;
		v = 0;
		s = n ^ v;

		SREG &= ~((1 << N) | (1 << Z) | (1 << C) | (1 << V) | (1 << S));
		SREG |= (n << N) | (z << Z) | (c << C) | (v << V) | (s << S);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_NEG:
		reg_n = (instruction >> 4) & 0x1F;
		tmp8 = Register8(reg_n);
		Register8(reg_n) = ~Register8(reg_n) + 1;
		h = ((1 << 3) & tmp8) | ((1 << 3) & Register8(reg_n));
		v = Register8(reg_n) == 0x80 ? 1 : 0;
		n = (1 << 7) & Register8(reg_n);
		z = Register8(reg_n) ? 0 : 1;
		c = !z;
		s = n ^ v;

		SREG &= ~((1 << H) | (1 << N) | (1 << Z) | (1 << C) | (1 << V) | (1 << S));
		SREG |= (h << H) | (n << N) | (z << Z) | (c << C) | (v << V) | (s << S);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_ASR:
		reg_n = (instruction >> 4) & 0x1F;
		c = Register8(reg_n) & 0x01;

		Register8(reg_n) = (Register8(reg_n) >> 1) | (Register8(reg_n) | (1 << 7));
		n = (Register8(reg_n) & 0x80) >> 7;
		z = Register8(reg_n) ? 0 : 1;
		v = n ^ c;
		s = n ^ v;

		SREG &= ~((1 << C) | (1 << N) | (1 << Z) | (1 << S) | (1 << V));
		SREG |= (c << C) | (n << N) | (z << Z) | (s << S) | (v << V);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_DEC:
		reg_n = (instruction >> 4) & 0x1F;
		Register8(reg_n) = Register8(reg_n) - 1;

		v = Register8(reg_n) == 0x80 ? 1 : 0;
		n = (Register8(reg_n) & 0x80) >> 7;
		z = Register8(reg_n) ? 0 : 1;
		s = n ^ v;

		SREG &= ~((1 << V) | (1 << N) | (1 << Z) | (1 << S));
		SREG |= (v << V) | (n << N) | (z << Z) | (s << S);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_INC:
		reg_n = (instruction >> 4) & 0x1F;
		Register8(reg_n) = Register8(reg_n) + 1;

		v = Register8(reg_n) == 0x80 ? 1 : 0;
		n = (Register8(reg_n) & 0x80) >> 7;
		z = Register8(reg_n) ? 0 : 1;
		s = n ^ v;

		SREG &= ~((1 << V) | (1 << N) | (1 << Z) | (1 << S));
		SREG |= (v << V) | (n << N) | (z << Z) | (s << S);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_LSR:
		reg_n = (instruction >> 4) & 0x1F;
		c = Register8(reg_n) & 0x01;
		Register8(reg_n) = Register8(reg_n) >> 1;
		z = Register8(reg_n) ? 0 : 1;
		n = 0;
		v = n ^ c;
		s = n ^ v;
		SREG &= ~((1 << C) | (1 << N) | (1 << Z) | (1 << S) | (1 << V));
		SREG |= (c << C) | (n << N) | (z << Z) | (s << S) | (v << V);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_ADD: // OP_LSL
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);

#ifdef _MSC_VER
		_addcarry_u8(0, Register8(Rd), Register8(Rr), &Register8(Rd));
#else
		__asm__ __volatile__(
		  "add %1, %0\n\t"
		  : "+r"(Register8(Rd))
		  : "r"(Register8(Rr))
		  : "cc");
#endif

		eflags = __readeflags();

		c = (eflags & (1 << CF));
		z = (eflags & (1 << ZF)) >> (ZF - Z);
		n = (eflags & (1 << SF)) >> (SF - N);
		v = (eflags & (1 << OF)) >> (OF - V);
		s = (n << 2) ^ (v << 1);
		h = (eflags & (1 << AF)) << (H - AF);

		SREG &= ~((1 << H) | (1 << S) | (1 << V) | (1 << N) | (1 << Z) | (1 << C));
		SREG |= h | s | v | n | z | c;

		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_ROR:
		reg_n = (instruction >> 4) & 0x1F;
		c = Register8(reg_n) & 0x01;
		Register8(reg_n) = (Register8(reg_n) >> 1) | ((SREG & 0x01) << 7);
		n = Register8(reg_n) & 0x80 ? 1 : 0;
		z = Register8(reg_n) ? 0 : 1;
		v = n ^ c;
		s = n ^ v;
		SREG &= ~((1 << C) | (1 << N) | (1 << Z) | (1 << S) | (1 << V));
		SREG |= (c << C) | (n << N) | (z << Z) | (s << S) | (v << V);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_ADC: // OP_ROL
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);

#ifdef _MSC_VER
		_addcarry_u8(GETF_CARRY(), Register8(Rd), Register8(Rr), &Register8(Rd));
#else
    __asm__ __volatile__(
      "bt %3, %2\n\t"
      "adc %1, %0\n\t"
      : "+r"(Register8(Rd))
      : "r"(Register8(Rr)),"r"((uint16_t)SREG), "i"(0)
      : "cc");
#endif
		eflags = __readeflags();

		c = (eflags & (1 << CF));
		z = (eflags & (1 << ZF)) >> (ZF - Z);
		n = (eflags & (1 << SF)) >> (SF - N);
		v = (eflags & (1 << OF)) >> (OF - V);
		s = (n << 2) ^ (v << 1);
		h = (eflags & (1 << AF)) << (H - AF);

		s = n ^ v;

		SREG &= ~((1 << H) | (1 << S) | (1 << V) | (1 << N) | (1 << Z) | (1 << C));
		SREG |= h | s | v | n | z | c;

		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SUB:
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);

#ifdef _MSC_VER
		_subborrow_u8(0, Register8(Rd), Register8(Rr), &Register8(Rd));
#else
        __asm__ __volatile__(
          "sub %1, %0\n\t"
          : "+r"(Register8(Rd))
          : "r"(Register8(Rr))
          : "cc");
#endif

		eflags = __readeflags();

		c = (eflags & (1 << CF));
		z = (eflags & (1 << ZF)) >> (ZF - Z);
		n = (eflags & (1 << SF)) >> (SF - N);
		v = (eflags & (1 << OF)) >> (OF - V);
		s = (n << 2) ^ (v << 1);
		h = (eflags & (1 << AF)) << (H - AF);

		SREG &= ~((1 << H) | (1 << S) | (1 << V) | (1 << N) | (1 << Z) | (1 << C));
		SREG |= h | s | v | n | z | c;

		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SBC:
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);
		_z = GETF_ZERO();

#ifdef _MSC_VER
		_subborrow_u8(GETF_CARRY(), Register8(Rd), Register8(Rr), &Register8(Rd));
#else
        __asm__ __volatile__(
          "bt %3, %2\n\t"
          "sbb %1, %0\n\t"
          : "+r"(Register8(Rd))
          : "r"(Register8(Rr)),"r"((uint16_t)SREG), "i"(0)
          : "cc");
#endif

		eflags = __readeflags();

		c = (eflags & (1 << CF));
		z = ((eflags & (1 << ZF)) >> (ZF - Z)) & _z;
		n = (eflags & (1 << SF)) >> (SF - N);
		v = (eflags & (1 << OF)) >> (OF - V);
		s = (n << 2) ^ (v << 1);
		h = (eflags & (1 << AF)) << (H - AF);

		SREG &= ~((1 << H) | (1 << S) | (1 << V) | (1 << N) | (1 << Z) | (1 << C));
		SREG |= h | s | v | n | z | c;

		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SUBI:
		Rd = ((instruction >> 4) & 0x0F) + 16;
		K = ((instruction >> 4) & 0xF0) | (instruction & 0x0F);

#ifdef _MSC_VER
		_subborrow_u8(0, Register8(Rd), K, &Register8(Rd));
#else
        __asm__ __volatile__(
          "sub %1, %0\n\t"
          : "+r"(Register8(Rd))
          : "r"(K)
          : "cc");
#endif

		eflags = __readeflags();

		c = (eflags & (1 << CF));
		z = ((eflags & (1 << ZF)) >> (ZF - Z));
		n = (eflags & (1 << SF)) >> (SF - N);
		v = (eflags & (1 << OF)) >> (OF - V);
		s = (n << 2) ^ (v << 1);
		h = (eflags & (1 << AF)) << (H - AF);

		SREG &= ~((1 << H) | (1 << S) | (1 << V) | (1 << N) | (1 << Z) | (1 << C));
		SREG |= h | s | v | n | z | c;

		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SBCI:
		Rd = ((instruction >> 4) & 0x0F) + 16;
		K = ((instruction >> 4) & 0xF0) | (instruction & 0x0F);
		_z = GETF_ZERO();

#ifdef _MSC_VER
		_subborrow_u8(GETF_CARRY(), Register8(Rd), K, &Register8(Rd));
#else
        __asm__ __volatile__(
          "bt %3, %2\n\t"
          "sbb %1, %0\n\t"
          : "+r"(Register8(Rd))
          : "r"(K),"r"((uint16_t)SREG), "i"(0)
          : "cc");
#endif

		eflags = __readeflags();

		c = (eflags & (1 << CF));
		z = ((eflags & (1 << ZF)) >> (ZF - Z)) & _z;
		n = (eflags & (1 << SF)) >> (SF - N);
		v = (eflags & (1 << OF)) >> (OF - V);
		s = (n << 2) ^ (v << 1);
		h = (eflags & (1 << AF)) << (H - AF);

		SREG &= ~((1 << H) | (1 << S) | (1 << V) | (1 << N) | (1 << Z) | (1 << C));
		SREG |= h | s | v | n | z | c;

		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_SWAP:
		Rd = (instruction >> 4) & 0x1F;
		Register8(Rd) = (Register8(Rd) << 4) | (Register8(Rd) >> 4);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_OR:
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);
		Register8(Rd) = Register8(Rd) | Register8(Rr);
		n = Register8(Rd) & 0x80 ? 1 : 0;
		z = Register8(Rd) ? 0 : 1;
		s = n ^ 0;
		SREG &= ~((1 << S) | (1 << V) | (1 << N) | (1 << Z));
		SREG |= ((s << S) | (n << N) | (z << Z));
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_AND:
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);
		Register8(Rd) = Register8(Rd) & Register8(Rr);
		n = Register8(Rd) & 0x80 ? 1 : 0;
		z = Register8(Rd) ? 0 : 1;
		s = n ^ 0;
		SREG &= ~((1 << S) | (1 << V) | (1 << N) | (1 << Z));
		SREG |= ((s << S) | (n << N) | (z << Z));
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_EOR:
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);
		Register8(Rd) = Register8(Rd) ^ Register8(Rr);
		n = Register8(Rd) & 0x80 ? 1 : 0;
		z = Register8(Rd) ? 0 : 1;
		s = n ^ 0;
		SREG &= ~((1 << S) | (1 << V) | (1 << N) | (1 << Z));
		SREG |= ((s << S) | (n << N) | (z << Z));
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_ORI: // case OP_SBR:
		Rd = ((instruction >> 4) & 0x0F) + 16;
		K = ((instruction >> 4) & 0xF0) | (instruction & 0x0F);
		Register8(Rd) = Register8(Rd) | K;
		n = Register8(Rd) & 0x80 ? 1 : 0;
		z = Register8(Rd) ? 0 : 1;
		s = n ^ 0;
		SREG &= ~((1 << S) | (1 << V) | (1 << N) | (1 << Z));
		SREG |= ((s << S) | (n << N) | (z << Z));
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_ANDI: // case OP_CBR:
		Rd = ((instruction >> 4) & 0x0F) + 16;
		K = ((instruction >> 4) & 0xF0) | (instruction & 0x0F);
		Register8(Rd) = Register8(Rd) & K;
		n = Register8(Rd) & 0x80 ? 1 : 0;
		z = Register8(Rd) ? 0 : 1;
		s = n ^ 0;
		SREG &= ~((1 << S) | (1 << V) | (1 << N) | (1 << Z));
		SREG |= ((s << S) | (n << N) | (z << Z));
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_MOV:
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);
		Register8(Rd) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_MOVW:
		Rd = ((instruction >> 4) & 0x0F) << 1;
		Rr = (instruction & 0x0F) << 1;
		Register8(Rd) = Register8(Rr);
		Register8(Rd + 1) = Register8(Rr + 1);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_ICALL:
		sram_.at(STACK) = (PC + 1) & 0xFF;
		sram_.at(STACK - 1) = ((PC + 1) >> 8) & 0xFF;
		STACK = STACK - 2;
		PC = (Register8(zh) << 8) | Register8(zl);
		CC = CC + 3;
		break;
	case OP_IJMP:
		PC = (Register8(zh) << 8) | Register8(zl);
		CC = CC + 2;
		break;
	case OP_RCALL:
		offset = (instruction & 0x0FFF);
		if (offset & (0x800)) {
			offset = (((~offset) & 0xFFF) + 1);
			offset = -offset;
		}
		sram_.at(STACK) = (PC + 1) & 0xFF;
		sram_.at(STACK - 1) = ((PC + 1) >> 8) & 0xFF;
		STACK = STACK - 2;
		CC = CC + 3;
		PC = PC + offset + 1;
		normalize_pc();
		break;
	case OP_RJMP:
		offset = (instruction & 0x0FFF);
		if (offset & (0x800)) {
			offset = (((~offset) & 0xFFF) + 1);
			offset = -offset;
		}
		CC = CC + 2;
		PC = PC + offset + 1;
		normalize_pc();
		break;
	case OP_CALL:
		sram_.at(STACK) = (PC + 2) & 0xFF;
		sram_.at(STACK - 1) = ((PC + 2) >> 8) & 0xFF;
		STACK = STACK - 2;
		PC = ((flash_.at(PC).instruction & 0x01) << 16) | ((flash_.at(PC).instruction & 0x1F0) << 13) | flash_.at(PC + 1).instruction;
		CC = CC + 4;
		break;
	case OP_JMP:
		PC = ((flash_.at(PC).instruction & 0x01) << 16) | ((flash_.at(PC).instruction & 0x1F0) << 13) | flash_.at(PC + 1).instruction;
		CC = CC + 3;
		break;
	case OP_RETI:
		SREG |= (1 << I);
		[[fallthrough]];
	case OP_RET:
		STACK = STACK + 2;
		PC = sram_.at(STACK - 1) << 8;
		PC |= sram_.at(STACK);
		CC = CC + 4;
		break;
	case OP_POP:
		Rd = (instruction >> 4) & 0x1F;
		STACK = STACK + 1;
		Register8(Rd) = sram_.at(STACK);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_PUSH:
		Rd = (instruction >> 4) & 0x1F;
		sram_.at(STACK) = Register8(Rd);
		STACK = STACK - 1;
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_CP:
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);

#ifdef _MSC_VER
		_subborrow_u8(0, Register8(Rd), Register8(Rr), &nu.u8);
#else
        __asm__ __volatile__(
          "sub %1, %0\n\t"
          :: "r"(Register8(Rd)), "r"(Register8(Rr))
          : "cc");
#endif

		eflags = __readeflags();

		c = (eflags & (1 << CF));
		z = ((eflags & (1 << ZF)) >> (ZF - Z));
		n = (eflags & (1 << SF)) >> (SF - N);
		v = (eflags & (1 << OF)) >> (OF - V);
		s = (n << 2) ^ (v << 1);
		h = (eflags & (1 << AF)) << (H - AF);

		SREG &= ~((1 << H) | (1 << S) | (1 << V) | (1 << N) | (1 << Z) | (1 << C));
		SREG |= h | s | v | n | z | c;

		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_CPC:
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);

		_z = GETF_ZERO();

#ifdef _MSC_VER
		_subborrow_u8(GETF_CARRY(), Register8(Rd), Register8(Rr), &nu.u8);
#else
        __asm__ __volatile__(
          "bt %3, %2\n\t"
          "sub %1, %0\n\t"
          :: "r"(Register8(Rd)), "r"(Register8(Rr)), "r"((uint16_t)SREG), "i"(0)
          : "cc");
#endif

		eflags = __readeflags();

		c = (eflags & (1 << CF));
		z = ((eflags & (1 << ZF)) >> (ZF - Z)) & _z;
		n = (eflags & (1 << SF)) >> (SF - N);
		v = (eflags & (1 << OF)) >> (OF - V);
		s = (n << 2) ^ (v << 1);
		h = (eflags & (1 << AF)) << (H - AF);

		SREG &= ~((1 << H) | (1 << S) | (1 << V) | (1 << N) | (1 << Z) | (1 << C));
		SREG |= h | s | v | n | z | c;

		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_CPI:
		Rd = ((instruction >> 4) & 0x0F) + 16;
		K = (instruction & 0x0F) | ((instruction & 0xF00) >> 4);

#ifdef _MSC_VER
		_subborrow_u8(0, Register8(Rd), K, &nu.u8);
#else
        __asm__ __volatile__(
          "sub %0, %1\n\t"
          :: "r"(Register8(Rd)), "r"(K)
          : "cc");
#endif

		eflags = __readeflags();

		c = (eflags & (1 << CF));
		z = ((eflags & (1 << ZF)) >> (ZF - Z));
		n = (eflags & (1 << SF)) >> (SF - N);
		v = (eflags & (1 << OF)) >> (OF - V);
		s = (n << 2) ^ (v << 1);
		h = (eflags & (1 << AF)) << (H - AF);

		SREG &= ~((1 << H) | (1 << S) | (1 << V) | (1 << N) | (1 << Z) | (1 << C));
		SREG |= h | s | v | n | z | c;

		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_CPSE:
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);
		if (Register8(Rd) != Register8(Rr)) {
			PC = PC + 1;
			CC = CC + 1;
		} else {
			int nextSize = flash_.at(PC + 1).size;
			PC = PC + nextSize + 1;
			CC = CC + nextSize + 1;
		}
		break;
	case OP_SBRC:
		bit_n = instruction & 0x07;
		reg_n = (instruction >> 4) & 0x1F;

		if (Register8(reg_n) & (1 << bit_n)) {
			PC = PC + 1;
			CC = CC + 1;
		} else {
			int nextSize = flash_.at(PC + 1).size;
			PC = PC + nextSize + 1;
			CC = CC + nextSize + 1;
		}
		break;
	case OP_SBRS:
		bit_n = instruction & 0x07;
		reg_n = (instruction >> 4) & 0x1F;

		if (Register8(reg_n) & (1 << bit_n)) {
			int nextSize = flash_.at(PC + 1).size;
			PC = PC + nextSize + 1;
			CC = CC + nextSize + 1;
		} else {
			PC = PC + 1;
			CC = CC + 1;
		}
		break;
	case OP_BLD:
		bit_n = instruction & 0x07;
		reg_n = (instruction >> 4) & 0x1F;

		if (SREG & (1 << T)) {
			Register8(reg_n) |= (1 << bit_n);
		} else {
			Register8(reg_n) &= ~(1 << bit_n);
		}
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_BST:
		bit_n = instruction & 0x07;
		reg_n = (instruction >> 4) & 0x1F;

		if (Register8(reg_n) & (1 << bit_n)) {
			SREG |= (1 << T);
		} else {
			SREG &= ~(1 << T);
		}
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_ADIW:
		Rd = ((instruction >> 4) & 0x03) * 2 + 24;
		K = (instruction & 0x0F) | ((instruction >> 2) & 0x30);

#ifdef _MSC_VER
		_addcarry_u16(0, Register16(Rd), (uint16_t)K, &Register16(Rd));
#else
    __asm__ __volatile__(
      "add %0, %1\n\t"
      : "+r"(Register16(Rd))
      : "r"((uint16_t)K)
      : "cc");
#endif

        eflags = __readeflags();

		c = (eflags & (1 << CF));
		z = ((eflags & (1 << ZF)) >> (ZF - Z));
		n = (eflags & (1 << SF)) >> (SF - N);
		v = (eflags & (1 << OF)) >> (OF - V);
		s = (n << 2) ^ (v << 1);

		SREG &= ~((1 << S) | (1 << V) | (1 << N) | (1 << Z) | (1 << C));
		SREG |= s | v | n | z | c;

		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_SBIW:
		Rd = ((instruction >> 4) & 0x03) * 2 + 24;
		K = (instruction & 0x0F) | ((instruction >> 2) & 0x30);

#ifdef _MSC_VER
		_subborrow_u16(0, Register16(Rd), (uint16_t)K, &Register16(Rd));
#else
    __asm__ __volatile__(
      "sub %0, %1\n\t"
      :: "r"(Register16(Rd)), "r"((uint16_t)K)
      : "cc");
#endif

		eflags = __readeflags();

		c = (eflags & (1 << CF));
		z = ((eflags & (1 << ZF)) >> (ZF - Z));
		n = (eflags & (1 << SF)) >> (SF - N);
		v = (eflags & (1 << OF)) >> (OF - V);
		s = (n << 2) ^ (v << 1);

		SREG &= ~((1 << S) | (1 << V) | (1 << N) | (1 << Z) | (1 << C));
		SREG |= s | v | n | z | c;

		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_BRCC: // case OP_BRSH:
		if (SREG & (1 << C)) {
			PC = PC + 1;
			CC = CC + 1;
		} else {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		}
		break;
	case OP_BRCS: // case OP_BRLO:
		if (SREG & (1 << C)) {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		} else {
			PC = PC + 1;
			CC = CC + 1;
		}
		break;
	case OP_BREQ:
		if (SREG & (1 << Z)) {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		} else {
			PC = PC + 1;
			CC = CC + 1;
		}
		break;
	case OP_BRNE:
		if (SREG & (1 << Z)) {
			PC = PC + 1;
			CC = CC + 1;
		} else {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		}
		break;
	case OP_BRGE:
		if (SREG & (1 << S)) {
			PC = PC + 1;
			CC = CC + 1;
		} else {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		}
		break;
	case OP_BRHC:
		if (SREG & (1 << H)) {
			PC = PC + 1;
			CC = CC + 1;
		} else {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		}
		break;
	case OP_BRHS:
		if (SREG & (1 << H)) {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		} else {
			PC = PC + 1;
			CC = CC + 1;
		}
		break;
	case OP_BRID:
		if (SREG & (1 << I)) {
			PC = PC + 1;
			CC = CC + 1;
		} else {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		}
		break;
	case OP_BRIE:
		if (SREG & (1 << I)) {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		} else {
			PC = PC + 1;
			CC = CC + 1;
		}
		break;
	case OP_BRLT:
		if (SREG & (1 << S)) {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		} else {
			PC = PC + 1;
			CC = CC + 1;
		}
		break;
	case OP_BRMI:
		if (SREG & (1 << N)) {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		} else {
			PC = PC + 1;
			CC = CC + 1;
		}
		break;
	case OP_BRPL:
		if (SREG & (1 << N)) {
			PC = PC + 1;
			CC = CC + 1;
		} else {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		}
		break;
	case OP_BRTC:
		if (SREG & (1 << T)) {
			PC = PC + 1;
			CC = CC + 1;
		} else {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		}
		break;
	case OP_BRTS:
		if (SREG & (1 << T)) {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		} else {
			PC = PC + 1;
			CC = CC + 1;
		}
		break;
	case OP_BRVC:
		if (SREG & (1 << V)) {
			PC = PC + 1;
			CC = CC + 1;
		} else {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		}
		break;
	case OP_BRVS:
		if (SREG & (1 << V)) {
			offset = (instruction >> 3) & 0x7F;
			PC = (offset & (1 << 6)) ? PC - (((~offset) & 0x7F) + 1) + 1 : PC + offset + 1;
			normalize_pc();
			CC = CC + 2;
		} else {
			PC = PC + 1;
			CC = CC + 1;
		}
		break;
	case OP_STS:
		reg_n = (instruction >> 4) & 0x1F;
		sram_.at(flash_.at(PC + 1).instruction) = Register8(reg_n);
		PC = PC + 2;
		CC = CC + 2;
		break;
	case OP_LDS:
		reg_n = (instruction >> 4) & 0x1F;
		Register8(reg_n) = sram_.at(flash_.at(PC + 1).instruction);
		PC = PC + 2;
		CC = CC + 2;
		break;
		// ST and STD
	case OP_ST_X:
		Rr = (instruction >> 4) & 0x1F;
		sram_.at(IndexRegX) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_ST_X_POST_INC:
		Rr = (instruction >> 4) & 0x1F;
		sram_.at(IndexRegX++) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_ST_X_PRE_DEC:
		Rr = (instruction >> 4) & 0x1F;
		sram_.at(--IndexRegX) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_ST_Y:
		Rr = (instruction >> 4) & 0x1F;
		sram_.at(IndexRegY) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_ST_Y_POST_INC:
		Rr = (instruction >> 4) & 0x1F;
		sram_.at(IndexRegY++) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_ST_Y_PRE_DEC:
		Rr = (instruction >> 4) & 0x1F;
		sram_.at(--IndexRegY) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_STD_Y:
		Rr = (instruction >> 4) & 0x1F;
		q = ((instruction & 8192) >> 8) | ((instruction & 3072) >> 7) | (instruction & 0x07);
		sram_.at(IndexRegY + q) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_ST_Z:
		Rr = (instruction >> 4) & 0x1F;
		sram_.at(IndexRegZ) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_ST_Z_POST_INC:
		Rr = (instruction >> 4) & 0x1F;
		sram_.at(IndexRegZ++) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_ST_Z_PRE_DEC:
		Rr = (instruction >> 4) & 0x1F;
		sram_.at(--IndexRegZ) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_STD_Z:
		Rr = (instruction >> 4) & 0x1F;
		q = ((instruction & 8192) >> 8) | ((instruction & 3072) >> 7) | (instruction & 0x07);
		sram_.at(IndexRegZ + q) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 2;
		break;
		// LD and LDD
	case OP_LD_X:
		Rr = (instruction >> 4) & 0x1F;
		Register8(Rr) = sram_.at(IndexRegX);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_LD_X_POST_INC:
		Rr = (instruction >> 4) & 0x1F;
		Register8(Rr) = sram_.at(IndexRegX++);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_LD_X_PRE_DEC:
		Rr = (instruction >> 4) & 0x1F;
		Register8(Rr) = sram_.at(--IndexRegX);
		PC = PC + 1;
		CC = CC + 3;
		break;
	case OP_LD_Y:
		Rr = (instruction >> 4) & 0x1F;
		Register8(Rr) = sram_.at(IndexRegY);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_LD_Y_POST_INC:
		Rr = (instruction >> 4) & 0x1F;
		Register8(Rr) = sram_.at(IndexRegY++);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_LD_Y_PRE_DEC:
		Rr = (instruction >> 4) & 0x1F;
		Register8(Rr) = sram_.at(--IndexRegY);
		PC = PC + 1;
		CC = CC + 3;
		break;
	case OP_LDD_Y:
		Rr = (instruction >> 4) & 0x1F;
		q = ((instruction & 8192) >> 8) | ((instruction & 3072) >> 7) | (instruction & 0x07);
		Register8(Rr) = sram_.at(IndexRegY + q);
		PC = PC + 1;
		CC = CC + 3;
		break;
	case OP_LD_Z:
		Rr = (instruction >> 4) & 0x1F;
		Register8(Rr) = sram_.at(IndexRegZ);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_LD_Z_POST_INC:
		Rr = (instruction >> 4) & 0x1F;
		Register8(Rr) = sram_.at(IndexRegZ++);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_LD_Z_PRE_DEC:
		Rr = (instruction >> 4) & 0x1F;
		Register8(Rr) = sram_.at(--IndexRegZ);
		PC = PC + 1;
		CC = CC + 3;
		break;
	case OP_LDD_Z:
		Rr = (instruction >> 4) & 0x1F;
		q = ((instruction & 8192) >> 8) | ((instruction & 3072) >> 7) | (instruction & 0x07);
		Register8(Rr) = sram_.at(IndexRegZ + q);
		PC = PC + 1;
		CC = CC + 3;
		break;
	case OP_SPM:
		PC = PC + 1;
		CC = CC + do_spm();
		break;
	case OP_LPM:
		tmp16 = IndexRegZ;
		if (tmp16 >= flash_.size() * 2)
			throw invalid_lpm_source();
		Register8(0) = flash_.get_byte(tmp16);
		PC = PC + 1;
		CC = CC + 3;
		break;
	case OP_LPM_R:
		Rd = (instruction >> 4) & 0x1F;
		tmp16 = IndexRegZ;
		if (tmp16 >= flash_.size() * 2)
			throw invalid_lpm_source();
		Register8(Rd) = flash_.get_byte(tmp16);
		PC = PC + 1;
		CC = CC + 3;
		break;
	case OP_LPM_R_POST_INC:
		Rd = (instruction >> 4) & 0x1F;
		tmp16 = IndexRegZ++;
		if (tmp16 >= flash_.size() * 2)
			throw invalid_lpm_source();
		Register8(Rd) = flash_.get_byte(tmp16);
		PC = PC + 1;
		CC = CC + 3;
		break;
	case OP_MUL:
		// 16 <= d <= 31, 16 <= r <= 31
		Rd = (instruction >> 4) & 0x1F;
		Rr = (instruction & 0x0F) | ((instruction & 0x200) >> 5);
		*static_cast<uint16_t*>(R0R1_PTR) = (uint8_t)Register8(Rd) * (uint8_t)Register8(Rr);

		c = (*((uint16_t*)R0R1_PTR) & (0x8000)) >> 15;
		z = *((uint16_t*)R0R1_PTR) ? 0 : 1;

		SREG &= ~((1 << Z) | (1 << C));
		SREG |= ((z << Z) | (c << C));

		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_MULS:
		// 16 <= d <= 31, 16 <= r <= 31
		Rd = ((instruction >> 4) & 0x0F) + 16;
		Rr = (instruction & 0x0F) + 16;
		*static_cast<int16_t*>(R0R1_PTR) = (int8_t)Register8(Rd) * (int8_t)Register8(Rr);

		c = (*((uint16_t*)R0R1_PTR) & (0x8000)) >> 15;
		z = *((uint16_t*)R0R1_PTR) ? 0 : 1;

		SREG &= ~((1 << Z) | (1 << C));
		SREG |= ((z << Z) | (c << C));

		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_MULSU:
		// 16 <= d <= 23, 16 <= r <= 23
		Rd = ((instruction >> 4) & 0x07) + 16;
		Rr = (instruction & 0x07) + 16;
		*static_cast<int16_t*>(R0R1_PTR) = (int8_t)Register8(Rd) * (uint8_t)Register8(Rr);

		c = (*((uint16_t*)R0R1_PTR) & (0x8000)) >> 15;
		z = *((uint16_t*)R0R1_PTR) ? 0 : 1;

		SREG &= ~((1 << Z) | (1 << C));
		SREG |= ((z << Z) | (c << C));

		PC = PC + 1;
		CC = CC + 2;
		break;
		// IO File
	case OP_SBIC:
		A = (instruction >> 3) & 0x1F;
		b = instruction & 0x07;
		if (!(IO_Register8(A) & (1 << b))) {
			int nextSize = flash_.at(PC + 1).size;
			PC = PC + nextSize + 1;
			CC = CC + nextSize + 1;
		} else {
			PC = PC + 1;
			CC = CC + 1;
		}
		break;
	case OP_SBIS:
		A = (instruction >> 3) & 0x1F;
		b = instruction & 0x07;
		if (IO_Register8(A) & (1 << b)) {
			int nextSize = flash_.at(PC + 1).size;
			PC = PC + nextSize + 1;
			CC = CC + nextSize + 1;
		} else {
			PC = PC + 1;
			CC = CC + 1;
		}
		break;
	case OP_CBI:
		A = (instruction >> 3) & 0x1F;
		b = instruction & 0x07;
		IO_Register8(A) &= ~(1 << b);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_SBI:
		A = (instruction >> 3) & 0x1F;
		b = instruction & 0x07;
		IO_Register8(A) |= (1 << b);
		PC = PC + 1;
		CC = CC + 2;
		break;
	case OP_IN:
		Rd = (instruction >> 4) & 0x1F;
		A = (instruction & 0x0F) | ((instruction & 0x600) >> 5);
		Register8(Rd) = IO_Register8(A);
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_OUT:
		Rr = (instruction >> 4) & 0x1F;
		A = (instruction & 0x0F) | ((instruction & 0x600) >> 5);
		IO_Register8(A) = Register8(Rr);
		PC = PC + 1;
		CC = CC + 1;
		break;
		//	case OP_SLEEP:
		//		break;
		//	case OP_BREAK:
		//		break;
	case OP_WDR:
		// do nothing for now
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_FAKE_UDR_OUT:
		*ucsra_ptr_ |= (1 << 6); // set TXC
		if (mstate_) {
			mstate_->uart_put_byte(Register8(instruction & 0x1F), dev_addr_);
			ev_list_.tx_in_progress = true;
		}
		PC = PC + 1;
		CC = CC + 1;
		break;
	case OP_FAKE_DEBUG:
	{
		int code = (instruction >> 3) & 0x07;
		int addr = (instruction & 0x07);

		if (code == 0) {
			std::cout << "ac: " << addr << std::endl;
		} else if (code == 2 || code == 1) {
			std::cout << addr << ((code == 2) ? " tx: " : " rx: ");
			auto frame = reinterpret_cast<net::Frame*>(&sram_[(addr == 2 ? 0xdd : 0xe9)]);
			std::cout << std::hex;
			for (int i = 0; i < frame->h.len; ++i) {
				std::cout << std::setw(2) << std::setfill('0')
					<< static_cast<int>(((uint8_t*)frame)[i]) << " ";
			}
			std::cout << std::dec << std::endl;
		} else if (code == 3) { 
			switch ((int)sram_[0x0060])
			{
			case DBG_NET_WRITE_PAGE_START:
				std::cout << "write page begin." << std::endl;
				break;
			case DBG_NET_WRITE_PAGE_END:
				std::cout << "write page end." << std::endl;
				break;
			default:
				std::cout << "unknown debug code." << std::endl;
				break;
			}
		//	std::cout << "a: " << addr << " c: " << code << 
		//		" v: " << (int)sram_[0x0060] << std::endl;
		}	
		PC = PC + 1;
		CC = CC + 1;
		break;
    }
	// sends a signal to the host
    case OP_FAKE_SIGNAL:
      if (signal_handler_) {
		  signal_handler_(sram_);
      }
	  PC = PC + 1;
	  CC = CC + 1;
	  break;
	default:
		throw unknown_insruction(instruction);
	}

	return CC - cc;
}

SIM_IMPORT_EXPORT void AVRCore::PrintLastInstructions() {
	std::ios::fmtflags f(std::cout.flags());
	std::cout << std::hex << std::setfill('0');
	std::cout << "\n SP:" << std::setw(4) << STACK;
	using cbuff = boost::circular_buffer<uint16_t>;
	using ri = typename cbuff::const_reverse_iterator;
	int k = 0;
	for (ri i = last_pc_.rbegin(); i != last_pc_.rend(); i++, k--) {
		std::cout << "\n (pc*2) " << std::setfill(' ') << std::setw(3) << std::dec << k << ": "
			<< std::hex << std::setfill('0') << std::setw(4) << (*i) * 2;
	}
	std::cout << std::endl;
	std::cout.flags(f);
}
