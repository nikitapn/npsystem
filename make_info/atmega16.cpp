// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <avr_info/avr_info.h>
#include "avr.h"

#define _AVR_IO_H_

#ifdef _MSC_VER
# pragma warning(disable:4068)  
#endif

#define __AVR_ATmega16__

#include "iom16.h"
#include "../avr_firmware/generated/m16.h"
#include "../avr_firmware/src/cfg/sconfig_m16.h"
#include "../avr_firmware/include/avr_firmware/twi.h"

MAKE_FIRMWARE_INFO(16)

#ifdef _MSC_VER
# pragma warning(default:4068)  
#endif


avrinfo::FirmwareInfo* info_m16 = &m16::info;