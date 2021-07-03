#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <memory>
#include <string_view>
#include <stdint.h>

namespace nplib::format {

template<typename T>
inline std::string to_hex(T val) noexcept {
	static_assert(std::is_arithmetic_v<T>, "T is not arithmetic type");
	std::stringstream ss;
	ss << "0x" << std::hex << std::setfill('0') << std::setw(sizeof(T)*2) << static_cast<uint64_t>(val);
	return ss.str();
}
template<typename T>
inline int from_hex(T& str) noexcept {
	int value;
	std::stringstream ss;
	ss << std::hex << str; ss >> value;
	return value;
}

template<typename ... Args>
std::string string_format(std::string_view format, Args ... args) {
	size_t size = snprintf(nullptr, 0, format.data(), args ...) + 1;
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.data(), args ...);
	return std::string(buf.get(), buf.get() + size - 1);
}

template<typename T>
inline std::string to_bit(T val) noexcept {
	static_assert(std::is_arithmetic_v<T>, "T is not arithmetic type");
	constexpr size_t length = sizeof(T) * 8;
	
	std::string res(length, ' ');
	for (size_t i = 0; i < length; ++i) {
		res[length - i - 1] = ((val >> i) & 0x01) + '0';
	}
	return res;
}

} // namespace nplib::format 