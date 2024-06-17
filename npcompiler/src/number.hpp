#pragma once

#include <cstdint>
#include <tuple>
#include <ostream>
#include <npsys/fundamental.h>

namespace npcompiler::fl {

// fundamental types
using FDType = npsys::nptype::Type;

struct Integer {
	std::uint64_t x;
	FDType type;
};

inline constexpr bool is_signed(int type) {
	return static_cast<bool>(type & npsys::nptype::SIGNED);
}

inline constexpr bool is_integer(int type) {
	return static_cast<bool>(type & npsys::nptype::INT_VALUE);
}

inline constexpr bool is_float(int type) {
	return static_cast<bool>(type & npsys::nptype::FLOAT_VALUE);
}

// size, signed
inline std::tuple<int, bool> get_number_info(int type) {
	return std::make_tuple((type & 0x0F) * 8, is_signed(type));
}

inline std::ostream& operator<<(std::ostream& os, const Integer& i) {
	if (is_signed(i.type)) os << '-';
	return os << i.x;
}

}