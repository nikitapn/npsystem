#include <nplib/utils/hexparser.h>
#include <fstream>
#include <iterator>

NPLIB_IMPORT_EXPORT
HexParser::HexParser(std::string_view filename) {
	std::ifstream file(filename.data());
	if (!file) throw std::runtime_error(std::string(filename) + " not found");
	buffer_ = std::string(
		std::istreambuf_iterator<char>(file), 
		std::istreambuf_iterator<char>()
	);
	buffer_ += '\0';
	hex_p_ = buffer_.c_str();
}

NPLIB_IMPORT_EXPORT
int HexParserAlgorithm::GetPageData(uint8_t* dest, int& size) {
	size = 0;
	if (*hex_p_ == '\0') return 0;

	unsigned char RecLen;
	unsigned char RecTyp;
	unsigned short LoadOffset;

	for (;;) {
		switch (state_) {
		case -1:
			if (*hex_p_++ == ':') {
				state_ = 0;
			} else {
				throw std::runtime_error("Wrong token");
			}
			break;
		case 0:
			RecLen = get_byte();
			if (RecLen == 0) {
				while (*++hex_p_ != '\0');
				if (size) {
					return 1;
				} else {
					return 0;
				}
			} else {
				linelenght_ = RecLen / 2;
				LoadOffset = get_word();
				RecTyp = get_byte();
				if (RecTyp == 3 || RecTyp == 2) {
					// ignore this shit
					while (*hex_p_++ != '\n');
					state_ = -1;
				} else {
					state_ = 1;
				}
			}
			break;
		case 1:
			while (linelenght_--) {
				unsigned short instruction = get_word();
				(*reinterpret_cast<unsigned short*>(dest + size)) = instruction;
				size += 2;
				if (size == page_size_)
					return 1;
			}
			// ignore crc
			while (*hex_p_++ != '\n');
			state_ = -1;
			break;
		};
	}
}

NPLIB_IMPORT_EXPORT
int HexParserAlgorithm::GetAlgSize() {
	unsigned char buffer[512];
	int size = 0, sum = 0;
	while (GetPageData(buffer, size))
		sum += size;
	hex_p_ = buffer_.c_str();
	state_ = -1;
	linelenght_ = 0;
	return sum;
}