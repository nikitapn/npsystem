// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "opcodes.h"
#ifdef _MSC_VER
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
#else
void InitModule() __attribute__((constructor));
#endif // _MSC_VER

const opcode_t instructions[] = {
	{ OP_CLC,		"1001010010001000", "clc" , 1 },
	{ OP_CLH,		"1001010011011000", "clh" , 1 },
	{ OP_CLI,		"1001010011111000", "cli" , 1 },
	{ OP_CLN,		"1001010010101000", "cln" , 1 },
	{ OP_CLS,		"1001010011001000", "cls" , 1 },
	{ OP_CLT,		"1001010011101000", "clt" , 1 },
	{ OP_CLV,		"1001010010111000", "clv" , 1 },
	{ OP_CLZ,		"1001010010011000", "clz" , 1 },
	{ OP_SEC,		"1001010000001000", "sec" , 1 },
	{ OP_SEH,		"1001010001011000", "seh" , 1 },
	{ OP_SEI,		"1001010001111000", "sei" , 1 },
	{ OP_SEN,		"1001010000101000", "sen" , 1 },
	{ OP_SES,		"1001010001001000", "ses" , 1 },
	{ OP_SET,		"1001010001101000", "set" , 1 },
	{ OP_SEV,		"1001010000111000", "sev" , 1 },
	{ OP_SEZ,		"1001010000011000", "sez" , 1 },
	{ OP_ICALL,		"1001010100001001", "icall" , 1 },
	{ OP_IJMP,		"1001010000001001", "ijmp" , 1 },
	{ OP_NOP,		"0000000000000000", "nop" , 1 },
	{ OP_RET,		"1001010100001000", "ret" , 1 },
	{ OP_RETI,		"1001010100011000", "reti" , 1 },
	{ OP_SLEEP,		"1001010110001000", "sleep" , 1 },
	{ OP_BREAK,		"1001010110011000", "break", 1 },
	{ OP_WDR,		"1001010110101000", "wdr" , 1 },
	{ OP_SPM,		"1001010111101000", "spm" , 1 },
	{ OP_ADC,		"000111rdddddrrrr", "adc r, r" , 1 },
	{ OP_ADD,		"000011rdddddrrrr", "add r, r" , 1 },
	{ OP_AND,		"001000rdddddrrrr", "and r, r", 1 },
	{ OP_CP,		"000101rdddddrrrr", "cp r, r" , 1 },
	{ OP_CPC,		"000001rdddddrrrr", "cpc r, r" , 1 },
	{ OP_CPSE,		"000100rdddddrrrr", "cpse r, r" , 1 },
	{ OP_EOR,		"001001rdddddrrrr", "eor r, r" , 1 },
	{ OP_OR,		"001010rdddddrrrr", "or r, r" , 1 },
	{ OP_SBC,		"000010rdddddrrrr", "sbc r, r" , 1 },
	{ OP_SUB,		"000110rdddddrrrr", "sub r, r" , 1 },
	{ OP_ANDI,		"0111KKKKddddKKKK", "andi d, M" , 1 },
	{ OP_LDI,		"1110KKKKddddKKKK", "ldi d, M" , 1 },
	{ OP_ORI,		"0110KKKKddddKKKK", "ori d, M" , 1 },
	{ OP_CPI,		"0011KKKKddddKKKK", "cpi d, M" , 1 },
	{ OP_SBCI,		"0100KKKKddddKKKK", "sbci d, M" , 1 },
	{ OP_SUBI,		"0101KKKKddddKKKK", "subi d, M" , 1 },
	{ OP_SBRC,		"1111110rrrrr0sss", "sbrc r, s" , 1 },
	{ OP_SBRS,		"1111111rrrrr0sss", "sbrs r, s" , 1 },
	{ OP_BLD,		"1111100ddddd0sss", "bld r, s" , 1 },
	{ OP_BST,		"1111101ddddd0sss", "bst r, s" , 1 },
	{ OP_IN,		"10110PPdddddPPPP", "in r, P" , 1 },
	{ OP_OUT,		"10111PPrrrrrPPPP", "out P, r" , 1 },
	{ OP_ADIW,		"10010110KKddKKKK", "adiw w, K" , 1 },
	{ OP_SBIW,		"10010111KKddKKKK", "sbiw w, K" , 1 },
	{ OP_CBI,		"10011000pppppsss", "cbi p, s" , 1 },
	{ OP_SBI,		"10011010pppppsss", "sbi p, s" , 1 },
	{ OP_SBIC,		"10011001pppppsss", "sbic p, s" , 1 },
	{ OP_SBIS,		"10011011pppppsss", "sbis p, s" , 1 },
	{ OP_BRCC,		"111101lllllll000", "brcc l" , 1 },
	{ OP_BRCS,		"111100lllllll000", "brcs l" , 1 },
	{ OP_BREQ,		"111100lllllll001", "breq l" , 1 },
	{ OP_BRGE,		"111101lllllll100", "brge l" , 1 },
	{ OP_BRHC,		"111101lllllll101", "brhc l" , 1 },
	{ OP_BRHS,		"111100lllllll101", "brhs l" , 1 },
	{ OP_BRID,		"111101lllllll111", "brid l" , 1 },
	{ OP_BRIE,		"111100lllllll111", "brie l" , 1 },
	{ OP_BRLT,		"111100lllllll100", "brlt l" , 1 },
	{ OP_BRMI,		"111100lllllll010", "brmi l" , 1 },
	{ OP_BRNE,		"111101lllllll001", "brne l" , 1 },
	{ OP_BRPL,		"111101lllllll010", "brpl l" , 1 },
	{ OP_BRTC,		"111101lllllll110", "brtc l" , 1 },
	{ OP_BRTS,		"111100lllllll110", "brts l" , 1 },
	{ OP_BRVC,		"111101lllllll011", "brvc l" , 1 },
	{ OP_BRVS,		"111100lllllll011", "brvs l" , 1 },
	{ OP_RCALL,		"1101LLLLLLLLLLLL", "rcall L" , 1 },
	{ OP_RJMP,		"1100LLLLLLLLLLLL", "rjmp L", 1 },
	{ OP_CALL,		"1001010hhhhh111h", "call h" , 2 },
	{ OP_JMP,		"1001010hhhhh110h", "jmp h" , 2 },
	{ OP_ASR,		"1001010rrrrr0101", "asr r" , 1 },
	{ OP_COM,		"1001010rrrrr0000", "com r" , 1 },
	{ OP_DEC,		"1001010rrrrr1010", "dec r" , 1 },
	{ OP_INC,		"1001010rrrrr0011", "inc r" , 1 },
	{ OP_LSR,		"1001010rrrrr0110", "lsr r" , 1 },
	{ OP_NEG,		"1001010rrrrr0001", "neg r" , 1 },
	{ OP_POP,		"1001000rrrrr1111", "pop r" , 1 },
	{ OP_PUSH,		"1001001rrrrr1111", "push r", 1 },
	{ OP_ROR,		"1001010rrrrr0111", "ror r" , 1 },
	{ OP_SWAP,		"1001010rrrrr0010", "swap r" , 1 },
	{ OP_MOV,		"001011rdddddrrrr", "mov r, r" , 1 },
	{ OP_MOVW,		"00000001ddddrrrr", "movw v, v" , 1 },
	{ OP_MUL,		"100111rdddddrrrr", "mul r, r" , 1 },
	{ OP_MULS,		"00000010ddddrrrr", "muls d, d" , 1 },
	{ OP_MULSU,		"000000110ddd0rrr", "mulsu a, a" , 1 },
	{ OP_STS,		"1001001ddddd0000", "sts i, r" , 2 },
	{ OP_LDS,		"1001000ddddd0000", "lds r, i" , 2 },
	{ OP_ST_X,			"1001001rrrrr1100", "st x, r" , 1 },
	{ OP_ST_X_POST_INC,	"1001001rrrrr1101", "st x+, r" , 1 },
	{ OP_ST_X_PRE_DEC,  "1001001rrrrr1110", "st -x, r" , 1 },
	{ OP_ST_Y,			"1000001rrrrr1000", "st y, r" , 1 },
	{ OP_ST_Y_POST_INC,	"1001001rrrrr1001", "st y+, r" , 1 },
	{ OP_ST_Y_PRE_DEC,	"1001001rrrrr1010", "st -y, r" , 1 },
	{ OP_STD_Y,			"10q0qq1rrrrr1qqq", "std y, r" , 1 },
	{ OP_ST_Z,			"1000001rrrrr0000", "st z, r" , 1 },
	{ OP_ST_Z_POST_INC,	"1001001rrrrr0001", "st z+, r" , 1 },
	{ OP_ST_Z_PRE_DEC,	"1001001rrrrr0010", "st -z, r" , 1 },
	{ OP_STD_Z,			"10q0qq1rrrrr0qqq", "std z, r" , 1 },
	{ OP_LD_X,			"1001000ddddd1100", "ld x, r" , 1 },
	{ OP_LD_X_POST_INC,	"1001000ddddd1101", "ld x+, r" , 1 },
	{ OP_LD_X_PRE_DEC,  "1001000ddddd1110", "ld -x, r" , 1 },
	{ OP_LD_Y,			"1000000ddddd1000", "ld y, r" , 1 },
	{ OP_LD_Y_POST_INC,	"1001000ddddd1001", "ld y+, r" , 1 },
	{ OP_LD_Y_PRE_DEC,	"1001000ddddd1010", "ld -y, r" , 1 },
	{ OP_LDD_Y,			"10q0qq0ddddd1qqq", "ldd y, r" , 1 },
	{ OP_LD_Z,			"1000000ddddd0000", "ld z, r" , 1 },
	{ OP_LD_Z_POST_INC,	"1001000ddddd0001", "ld z+, r" , 1 },
	{ OP_LD_Z_PRE_DEC,	"1001000ddddd0010", "ld -z, r" , 1 },
	{ OP_LDD_Z,			"10q0qq0ddddd0qqq", "ldd Rd, z+q" , 1 },
	{ OP_LPM,			"1001010111001000", "lpm" , 1 },
	{ OP_LPM_R,			"1001000ddddd0100", "lpm Rd, Z" , 1 },
	{ OP_LPM_R_POST_INC,"1001000ddddd0101", "lpm Rd, Z+" , 1 },
	{ OP_EICALL,		"1001010100011001", "eicall" , 1 },
	{ OP_EIJMP,			"1001010000011001", "eijmp" , 1 },
	{ OP_FAKE_UDR_OUT,	"00000000001rrrrr", "fake_udr_out", 1 },
	{ OP_FAKE_DEBUG,	"0000000001rrraaa", "fake_debug", 1 },
	{ OP_UNKNOWN,		"xxxxxxxxxxxxxxxx", "unknown", 0 }
};

uint16_t masks[OP_UNKNOWN];
uint16_t patterns[OP_UNKNOWN];

void InitDll()
{
	constexpr auto sz = sizeof(instructions) / sizeof(opcode_t);

	auto is_binary = [](char c) -> bool {
		int diff = (int)c - (int)'0';
		return ((diff >= 0) && (diff <= 1)) ? true : false; };

	for (int i = 0; (instructions[i].opcode != OP_UNKNOWN); i++) {
		uint16_t mask = 0;
		uint16_t pattern = 0;
		for (int k = 0, j = 15; k < 16; k++, j--) {
			if (is_binary(instructions[i].op[k])) {
				mask |= (1 << j);
				pattern |= ((instructions[i].op[k] - '0') << j);
			}
		}
		masks[i] = mask;
		patterns[i] = pattern;
	}
}


#ifdef _MSC_VER
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		InitDll();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#else
void InitModule()
{
	InitDll();
}
#endif