#pragma once

#include <cstdint>
#include <tuple>
#include <ostream>
#include <npsys/variable.h>

namespace npcompiler::fl {

enum FDType {
	FDT_BOOLEAN = static_cast<int>(npsys::variable::Type::VT_DISCRETE),
	FDT_U8 = static_cast<int>(npsys::variable::Type::VT_BYTE),
	FDT_S8 = static_cast<int>(npsys::variable::Type::VT_SIGNED_BYTE),
	FDT_U16 = static_cast<int>(npsys::variable::Type::VT_SIGNED_WORD),
	FDT_S16 = static_cast<int>(npsys::variable::Type::VT_WORD),
	FDT_U32 = static_cast<int>(npsys::variable::Type::VT_SIGNED_DWORD),
	FDT_S32 = static_cast<int>(npsys::variable::Type::VT_DWORD),
	FDT_F32 = static_cast<int>(npsys::variable::Type::VT_FLOAT)
};

struct Integer {
	std::uint64_t x;
	FDType type;
};

inline constexpr bool is_signed(int type) {
	return static_cast<bool>(type & npsys::variable::SIGNED);
}

inline constexpr bool is_integer(int type) {
	return static_cast<bool>(type & npsys::variable::INT_VALUE);
}

inline constexpr bool is_float(int type) {
	return static_cast<bool>(type & npsys::variable::FLOAT_VALUE);
}

// size, signed
inline std::tuple<int, bool> get_number_info(int type) {
	return std::make_tuple((type & 0x0F) * 8, is_signed(type));
}

inline std::ostream& operator<<(std::ostream& os, const Integer& i) {
	return os << (is_signed(i.type) ? -i.x : i.x);
}





}