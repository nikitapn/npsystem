// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <avr_info/avr_info.h>
#include "../../avr_firmware/.out/generated/pc-link-virtual.h"

namespace {

avrinfo::VirtualPCLinkInfo info = {
	V_DATA_ADDR,
	V_DATA_RECIEVED,
	V_DATA_TRASMITTED,
	V_BUSY
};

}
avrinfo::VirtualPCLinkInfo* info_pclink_virtual = &info;


