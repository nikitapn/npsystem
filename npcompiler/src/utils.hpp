#pragma once


#include <cstdint>
#include <string_view>
#include <charconv>
#include <cassert>
#include <tuple>

#include "number.hpp"

namespace npcompiler::utils {

inline fl::Integer parse_integer(std::string_view str) noexcept {
	using namespace npsys::nptype;

	std::int64_t x;
	
	auto result = std::from_chars(str.data(), str.data() + str.length(), x);
	assert(result.ec == std::errc());

	std::uint64_t u = std::abs(x);

	int type = 0;
	if (x < 0) type |= npsys::nptype::SIGNED;

	if (!(u & (~0x01)) && x > 0) type = NPT_BOOL;
	else if (!(u & (~0xFF))) type |= SIZE_8 | INT_VALUE;
	else if (!(u & (~0xFFFF))) type |= SIZE_16 | INT_VALUE;
	else type |= SIZE_32 | INT_VALUE;
	
	return { u, static_cast<fl::FDType>(type) };
}

}