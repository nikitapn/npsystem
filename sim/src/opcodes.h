// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

enum AVR_OPCODE : uint8_t {
	OP_CLC,
	OP_CLH,
	OP_CLI,
	OP_CLN,
	OP_CLS,
	OP_CLT,
	OP_CLV,
	OP_CLZ,
	OP_SEC,
	OP_SEH,
	OP_SEI,
	OP_SEN,
	OP_SES,
	OP_SET,
	OP_SEV,
	OP_SEZ,
	OP_ICALL,
	OP_IJMP,
	OP_NOP,
	OP_RET,
	OP_RETI,
	OP_SLEEP,
	OP_BREAK,
	OP_WDR,
	OP_SPM,
	OP_ADC,
	OP_ADD,
	OP_AND,
	OP_CP,
	OP_CPC,
	OP_CPSE,
	OP_EOR,
	OP_OR,
	OP_SBC,
	OP_SUB,
	OP_ANDI,
	OP_LDI,
	OP_ORI,
	OP_CPI,
	OP_SBCI,
	OP_SUBI,
	OP_SBRC,
	OP_SBRS,
	OP_BLD,
	OP_BST,
	OP_IN,
	OP_OUT,
	OP_ADIW,
	OP_SBIW,
	OP_CBI,
	OP_SBI,
	OP_SBIC,
	OP_SBIS,
	OP_BRCC,
	OP_BRCS,
	OP_BREQ,
	OP_BRGE,
	OP_BRHC,
	OP_BRHS,
	OP_BRID,
	OP_BRIE,
	OP_BRLT,
	OP_BRMI,
	OP_BRNE,
	OP_BRPL,
	OP_BRTC,
	OP_BRTS,
	OP_BRVC,
	OP_BRVS,
	OP_RCALL,
	OP_RJMP,
	OP_CALL,
	OP_JMP,
	OP_ASR,
	OP_COM,
	OP_DEC,
	OP_INC,
	OP_LSR,
	OP_NEG,
	OP_POP,
	OP_PUSH,
	OP_ROR,
	OP_SWAP,
	OP_MOV,
	OP_MOVW,
	OP_MUL,
	OP_MULS,
	OP_MULSU,
	OP_STS,
	OP_LDS,
	OP_ST_X,
	OP_ST_X_POST_INC,
	OP_ST_X_PRE_DEC,
	OP_ST_Y,
	OP_ST_Y_POST_INC,
	OP_ST_Y_PRE_DEC,
	OP_STD_Y,
	OP_ST_Z,
	OP_ST_Z_POST_INC,
	OP_ST_Z_PRE_DEC,
	OP_STD_Z,
	OP_LD_X,
	OP_LD_X_POST_INC,
	OP_LD_X_PRE_DEC,
	OP_LD_Y,
	OP_LD_Y_POST_INC,
	OP_LD_Y_PRE_DEC,
	OP_LDD_Y,
	OP_LD_Z,
	OP_LD_Z_POST_INC,
	OP_LD_Z_PRE_DEC,
	OP_LDD_Z,
	OP_LPM,
	OP_LPM_R,
	OP_LPM_R_POST_INC,
	OP_EICALL,
	OP_EIJMP,
	OP_FAKE_UDR_OUT,
	OP_FAKE_DEBUG,
	OP_FAKE_SIGNAL,
	OP_UNKNOWN,
};

struct opcode_t {
	AVR_OPCODE opcode;
	const char op[17];
	const char mnemonic[15];
	uint8_t size;
};

struct io_reg_t
{
	char address[10];
	char name[10];
};

extern uint16_t masks[];
extern uint16_t patterns[];
extern const opcode_t instructions[];
