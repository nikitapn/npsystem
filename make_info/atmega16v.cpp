#include <avr_info/avr_info.h>
#include "avr.h"

#define _AVR_IO_H_

#ifdef _MSC_VER
# pragma warning(disable:4068)  
#endif

#define __AVR_ATmega16__

#include "iom16.h"
#include "../avr_firmware/generated/m16v.h"
#include "../avr_firmware/src/cfg/sconfig_m16.h"
#include "../avr_firmware/include/avr_firmware/twi.h"

MAKE_FIRMWARE_INFO(16v)

#ifdef _MSC_VER
# pragma warning(default:4068)  
#endif


avrinfo::FirmwareInfo* info_m16v = &m16v::info;