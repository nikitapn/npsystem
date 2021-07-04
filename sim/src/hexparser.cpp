// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "hexparserflash.h"

SIM_IMPORT_EXPORT
void HexParserFlash::Read(Flash& flash) {
	uint8_t RecLen, RecTyp;
	uint16_t instruction;
	int offset, state = -1;
	for (;;)
	{
		switch (state)
		{
		case -1:
			if ( *hex_p_ == ':' ) {
				state = 0;
			} else if ( *hex_p_ == '\0') {
				hex_p_ = buffer_.c_str();
				return;
			} else {
				throw std::runtime_error("Wrong token");
			}
			hex_p_++;
			break;
		case 0:
			RecLen = get_byte();
			if (RecLen == 0 ) {
				while (*hex_p_++ != '\n');
				state = -1;
			} else {
				offset = get_byte() << 8;
				offset |= get_byte();
				RecTyp = get_byte();
				if (RecTyp == 3 || RecTyp == 2) {
					// pass this
					while (*hex_p_++ != '\n');
					state = -1;
				} else {
					state = 1;
				}
			}
			break;
		case 1:
			for (int i = 0; i < RecLen; i += 2) {
				instruction = get_word();
				flash.Store(offset + i, instruction);
			}
			// pass crc
			while ( *hex_p_++ != '\n' );
			state = -1;
			break;
		};
	}
}