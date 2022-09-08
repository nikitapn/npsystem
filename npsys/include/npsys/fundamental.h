#pragma once

#include <cassert>

namespace npsys::nptype {

static constexpr int VQUALITY			= 0x00002000;
static constexpr int IO_SPACE			= 0x00004000;
static constexpr int MUTABLE			= 0x00008000;
static constexpr int SIGNED				= 0x00001000;
static constexpr int INTERNAL			= 0x00010000;
static constexpr int SIZE_8				= 0x00000001;
static constexpr int SIZE_16			= 0x00000002;
static constexpr int SIZE_32			= 0x00000004;
static constexpr int FLOAT_VALUE	= 0x00000010;
static constexpr int BIT_VALUE		= 0x00000020;
static constexpr int INT_VALUE		= 0x00000040;
static constexpr int BIT_MASK			= 0x00000F00;
static constexpr int SIZE_MASK		= 0x0000000F;
static constexpr int TYPE_MASK = BIT_VALUE | INT_VALUE | FLOAT_VALUE | SIGNED | SIZE_8 | SIZE_16 | SIZE_32;

enum Type {
	NPT_UNDEFINE = 0x00000000,
	NPT_BOOL	= SIZE_8 | BIT_VALUE,
	NPT_U8		= SIZE_8 | INT_VALUE,
	NPT_I8		= SIGNED | SIZE_8 | INT_VALUE,
	NPT_U16		= SIZE_16 | INT_VALUE,
	NPT_I16		= SIGNED | SIZE_16 | INT_VALUE,
	NPT_U32		= SIZE_32 | INT_VALUE,
	NPT_I32		= SIGNED | SIZE_32 | INT_VALUE,
	NPT_F32		= SIGNED | SIZE_32 | FLOAT_VALUE
};

inline int compare(int a, int b) {
	if (a == b) return 0;
	
	const auto sz_a = (a & SIZE_MASK);
	const auto sz_b = (b & SIZE_MASK);

	if (sz_a == sz_b) {
		if (a & BIT_VALUE) return -1;
		if ((a & FLOAT_VALUE) && (b & INT_VALUE)) return 1;
		if ((a & INT_VALUE) && (b & FLOAT_VALUE)) return -1;
		
		if ((a & SIGNED) && !(b & SIGNED)) return 1;
		if (!(a & SIGNED) && (b & SIGNED)) return -1;

		assert(false);

		return 0x1111;
	} else if (sz_a > sz_b) {
		return 1;
	} else {
		return -1;
	}
}

} // namespace npsystem::nptype