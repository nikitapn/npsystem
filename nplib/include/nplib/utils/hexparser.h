#pragma once

#include <string>
#include <string_view>
#include "../import_export.h"

class HexParser {
public:
	NPLIB_IMPORT_EXPORT HexParser(std::string_view filename);
protected:
	uint8_t get_tet();
	uint8_t get_byte();
	uint16_t get_word();

	std::string buffer_;
	const char* hex_p_ = nullptr;
	int	offset_ = offset_;
	int pageSize_;
	int	flashSize_;
};

inline uint8_t HexParser::get_tet() {
	uint8_t b;
	if (*hex_p_ < 'A') b = (*hex_p_ - '0');
	else b = (*hex_p_ - 'A') + 0x0A;
	hex_p_++;
	return b;
}

inline uint8_t HexParser::get_byte() {
	uint8_t val = get_tet() << 4;
	return val | get_tet();
}

inline uint16_t HexParser::get_word() {
	uint16_t val = get_byte();
	return val | (get_byte() << 8);
}

class HexParserAlgorithm : public HexParser {
public:
	HexParserAlgorithm(std::string_view filename, int page_size)
		: HexParser(filename)
		, page_size_(page_size) {}

	NPLIB_IMPORT_EXPORT int GetPageData(uint8_t* dest, int& size);
	NPLIB_IMPORT_EXPORT int GetAlgSize();
private:
	int state_ = -1;
	int linelenght_ = 0;
	const int page_size_;
};
