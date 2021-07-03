#pragma once

#include <avr_info/avr_info.h>

#define STACK_SIZE 200

#define MAKE_FIRMWARE_INFO(chip)\
namespace m ## chip{\
avrinfo::FirmwareInfo info;\
void init(int version) {\
	info.version = version;\
	info.rmem =\
	{\
		RAMEND,\
		SYS_NOINIT_START,\
		SYS_RAM_END,\
		RAMEND - STACK_SIZE,\
		INFO_START,\
		TT_RAM,\
		RD_RAM,\
		TWI_RAM,\
		ADC_ARRAY,\
		EEPRCFG,\
		INTERNAL_TIME\
	};\
\
	info.emem =\
	{\
		EEPROM_EEPRCFG,\
		USER_EEPROM_START\
	};\
\
	info.fmem = \
	{\
		TT_FLASH,\
		RD_FLASH,\
		IO_CONFIG,\
		TWI_FLASH,\
		USER_FLASH_START,\
		USER_FLASH_END\
	}; \
\
	info.pmem = \
	{\
		SPM_PAGESIZE,\
		(FLASHEND + 1) / SPM_PAGESIZE,\
		TT_FLASH / SPM_PAGESIZE,\
		RD_FLASH / SPM_PAGESIZE,\
		IO_CONFIG / SPM_PAGESIZE,\
		TWI_FLASH / SPM_PAGESIZE,\
		USER_FLASH_START / SPM_PAGESIZE,\
		USER_FLASH_END / SPM_PAGESIZE,\
		(USER_FLASH_END - USER_FLASH_START) / SPM_PAGESIZE\
	};\
\
	info.mccfg =\
	{\
		PARTNO,\
		ADC_MAX,\
		R_DATA_MAX,\
		0,\
		0.010048f,\
		TWI_USEFULL_BUFFER_MAX,\
		TWI_SEGMENTS_MAX\
	};\
\
}\
}