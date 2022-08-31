// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <charconv>
#include <limits>
#include <stdexcept>

class input_error : public std::runtime_error {
public:
	explicit input_error(const char* s) 
		: std::runtime_error(s) {}

	explicit input_error(const std::string& s) 
		: std::runtime_error(s) {}
};

inline bool IsNumber(const char* lpsz, size_t len) noexcept {
	if (lpsz == nullptr) return false;
	if (len == -1) len = strlen(lpsz);
	if (len == 0) return false;

	const char* p;

	if (len > 2 && (lpsz[0] == '0' && lpsz[1] == 'x')) {
		p = lpsz + 2;
		while (*p) {
			auto ch = *p++;
			if (!((ch >= 'A' && ch <= 'F') || 
				(ch >= 'a' && ch <= 'f') || 
				(ch >= '0' && ch <= '9'))
				) return false;
		}
	} else if (len > 1 && (lpsz[0] == '-' || lpsz[0] == '+')) {
		p = lpsz + 1;
		while (*p) {
			auto ch = *p++;
			if (ch < '0' || ch > '9')
				return false;
		}
	} else {
		p = lpsz;
		while (*p) {
			auto ch = *p++;
			if (ch < '0' || ch > '9') return false;
		}
	}
	return true;
}

template<typename T>
void ParseTextValue(const char* lpsz, size_t len, T& value) {
	if (!IsNumber(lpsz, len)) throw input_error("The data have incorrect format");

	if constexpr (!std::numeric_limits<T>::is_signed) {
		if (lpsz[0] == '-') throw input_error("The data have incorrect format");
	}

	std::from_chars_result result;
	uint64_t tmp = 0;

	if (len > 1 && lpsz[0] == 'x') {
		result = std::from_chars(lpsz + 1, lpsz + len, tmp, 16);
	} else if (len > 2 && lpsz[0] == '0' && lpsz[1] == 'x') {
		result = std::from_chars(lpsz + 2, lpsz + len, tmp, 16);
	} else {
		result = std::from_chars(lpsz, lpsz + len, tmp, 10);
	}

	if (result.ec != std::errc()) throw input_error("The data have incorrect format");

	if constexpr (std::numeric_limits<T>::is_signed) {
		if (static_cast<int64_t>(tmp) > static_cast<int64_t>(std::numeric_limits<T>::max()) ||
			static_cast<int64_t>(tmp) < static_cast<int64_t>(std::numeric_limits<T>::min())) {
			throw input_error("The data have incorrect format");
		}
	} else {
		if (tmp > static_cast<uint64_t>(std::numeric_limits<T>::max())) {
			throw input_error("The data have incorrect format");
		}
	}

	value = static_cast<T>(tmp);
}

template<>
inline void ParseTextValue(const char* lpsz, size_t len, bool& value) {
	if (len == 0) throw input_error("The data have incorrect format");

	if ((len == 1) && (lpsz[0] == L'0' || lpsz[0] == L'1')) {
		value = static_cast<bool>(lpsz[0] - L'0');
		return;
	}

	if ((len == (sizeof("TRUE") / sizeof(char))) && (strcmp(lpsz, "TRUE") == 0)) {
		value = true;
		return;
	}

	if ((len == (sizeof("FALSE") / sizeof(char))) && (strcmp(lpsz, "FALSE") == 0)) {
		value = false;
		return;
	}

	throw input_error("The data have incorrect format");
}

template<>
inline void ParseTextValue(const char* lpsz, size_t len, float& value) {
	for (size_t i = 0; i < len; ++i) {
		if (lpsz[i] == L',') {
			throw input_error("The data have incorrect format\r\nUse \".\" for fractional part");
		}
	}
#ifdef _MSC_VER
	auto [ptr, ec] = std::from_chars(lpsz, lpsz + len, value);
	if (ec == std::errc()) return;
#else
	if (sscanf(lpsz, "f", &value) != EOF) return;
#endif
	throw input_error("The data have incorrect format");
}

template<>
inline void ParseTextValue(const char* lpsz, size_t len, double& value) {
	for (size_t i = 0; i < len; ++i) {
		if (lpsz[i] == L',') {
			throw input_error("The data have incorrect format\r\nUse \".\" for fractional part");
		}
	}
#ifdef _MSC_VER
	auto [ptr, ec] = std::from_chars(lpsz, lpsz + len, value);
	if (ec == std::errc()) return;
#else
	if (sscanf(lpsz, "lf", &value) != EOF) return;
#endif
	throw input_error("The data have incorrect format");
}