#include <avr_info/avr_info.h>
#include "../avr_firmware/generated/pc-link-virtual.h"

namespace {

avrinfo::VirtualPCLinkInfo info = {
	V_DATA_ADDR,
	V_DATA_RECIEVED,
	V_DATA_TRASMITTED,
	V_BUSY
};

}
avrinfo::VirtualPCLinkInfo* info_pclink_virtual = &info;


