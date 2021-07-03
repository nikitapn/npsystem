#include <avr_info/avr_info.h>
#include "avr.h"

#define _AVR_IO_H_

#ifdef _MSC_VER
# pragma warning(disable:4068)  
#endif

#define __AVR_ATmega8__

#include "iom8.h"
#include "../avr_firmware/generated/m8v.h"
#include "../avr_firmware/src/cfg/sconfig_m8.h"
#include "../avr_firmware/include/avr_firmware/twi.h"

MAKE_FIRMWARE_INFO(8v)

#ifdef _MSC_VER
# pragma warning(default:4068)  
#endif


avrinfo::FirmwareInfo* info_m8v = &m8v::info;