// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <avr_info/avr_info.h>
#include "avr.h"

#define _AVR_IO_H_

#ifdef _MSC_VER
# pragma warning(disable:4068)  
#endif

#define __AVR_ATmega8__

#include "iom8.h"
#include "../../avr_firmware/.out/generated/m8.h"
#include "../../avr_firmware/src/cfg/sconfig_m8.h"
#include "../../avr_firmware/include/avr_firmware/twi.h"

MAKE_FIRMWARE_INFO(8)

#ifdef _MSC_VER
# pragma warning(default:4068)  
#endif


avrinfo::FirmwareInfo* info_m8 = &m8::info;
