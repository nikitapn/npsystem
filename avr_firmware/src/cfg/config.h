#ifndef __NET_CONFIG_H__
#define __NET_CONFIG_H__

#if defined(__AVR_ATmega8__)
	#include "hconfig_m8.h"
	#include "sconfig_m8.h"
#elif defined(__AVR_ATmega16__)
	#include "hconfig_m16.h"
	#include "sconfig_m16.h"
#else
	#error "Chip isn't defined"
#endif

#if F_CPU == 11059200UL
	#error "11059200UL"
	#define PROPAGATION_DELAY	5
	#define IBBPC_1				2
	#define IBBPC_3				8
	#define IBBPC_4				11
	#define IBBPC_8				23
	//#define IBBPC_10			29
	#define IBBPC_10			89
	//#define IBBPC_30			89
	#define IBBPC_30			114
	//#define IBBPC_40			119
	#define IBBPC_40			150
#elif F_CPU == 16000000UL
	#define PROPAGATION_DELAY	10
	#define IBBPC_1				3
	#define IBBPC_3				9
	#define IBBPC_4				12
	#define IBBPC_8				35
	#define IBBPC_10			100
	#define IBBPC_30			140
	#define IBBPC_40			170
#else
	#error "F_CPU isn't defined"
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define MAKE_DEBUG_CMD(dev_addr, cmd) \
	asm(".long " TOSTRING(0x0040 | dev_addr | (cmd << 3)))

#define MAKE_DEBUG_CMD_0060(dev_addr, val) \
	do {*(uint8_t*)(0x0060) = val; \
	MAKE_DEBUG_CMD(dev_addr, 3);} while(0)

#endif // __NET_CONFIG_H__