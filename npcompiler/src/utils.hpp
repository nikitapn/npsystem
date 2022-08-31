#pragma once


#include <cstdint>
#include <string_view>
#include <charconv>
#include <cassert>
#include <tuple>

#include "number.hpp"

namespace npcompiler::utils {

inline fl::Integer parse_integer(std::string_view str) noexcept {
	std::int64_t x;
	
	auto result = std::from_chars(str.data(), str.data() + str.length(), x);
	assert(result.ec == std::errc());

	std::uint64_t u = std::abs(x);

	int type = 0;
	if (x < 0) type |= npsys::variable::SIGNED;

	if (!(u & (~0x01)) && x > 0) type = fl::FDT_BOOLEAN;
	else if (!(u & (~0xFF))) type |= npsys::variable::SIZE_8 | npsys::variable::INT_VALUE;
	else if (!(u & (~0xFFFF))) type |= npsys::variable::SIZE_16 | npsys::variable::INT_VALUE;
	else type |= npsys::variable::SIZE_32 | npsys::variable::INT_VALUE;
	
	return { u, static_cast<fl::FDType>(type) };
}

}