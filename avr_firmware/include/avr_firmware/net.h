// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#ifndef NET_H_
#define NET_H_


#ifdef NPSYSTEM_FIRMWARE_API_CPP
	#include <stdint.h>
namespace net {
#endif // c

#define NET_WRITE_PAGE_FAIL			((uint8_t)-1)
#define NET_ALGORTIHM_NOT_FOUND		((uint8_t)-2)

#ifndef __ASSEMBLER__

	/*
	bytes order - little endian;
	data_rw_type == 1 - read mode,
	data_rw_type == 0 - write mode;
	fun is a combination of REQ_TYPE | bit_value
	Read bytes (0x02):
	req:	|	 0    |    1   |  2  |        4			|      4       |       5      |   6   |   7   |
			| sl_addr | m_addr | fun | nbytes + nstatus | addr_start_l | addr_start_h | crc_l | crc_h |

	answ:	|	 0    |    1   |  2  |        4			|    5   |    6     |
			| sl_addr | m_addr | fun | nbytes + nstatus | byte_0 | status_0 | ... | byte_n | status_n | crc_l | crc_h |

	Read bits (0x03):
	req:	|	 0    |    1   |  2  |   4	  |        5     |       6      |   6   |   7   |
			| sl_addr | m_addr | fun | nbytes | addr_start_l | addr_start_h | crc_l | crc_h |

	answ:	|	 0    |    1   |  2  |   4	  |    5   |
			| sl_addr | m_addr | fun | nbytes | byte_0 | ... | byte_n | crc_l | crc_h |

	Write bytes (0x00):
	value_sz can be : 1, 2, 4 (u8, u16, u32)
	req:
			| sl_addr | m_addr | fun | req_size | value_sz | addr_start_l | addr_start_h | value_lsb | ... | value_msb | status | ... | crc_l | crc_h |
	answ	| sl_addr | m_addr | fun | req_size | value_sz | addr_start_l | addr_start_h | value_lsb | ... | value_msb | status | ... | crc_l | crc_h |
	Write bits:
	fun: 0x01
	| sl_addr | m_addr | fun | req_size | nbits | addr_l_0 | addr_h_0 | bitn_0 | value_0 | status_0 | ... | addr_l_n | addr_h_n | bitn_n | value_n | status_n | crc_l | crc_h |

	*/

	//#define BAUDRATE	9600UL
	//#define BAUDRATE	57600UL
#define BAUDRATE	76800UL

#define R_DATA_PERIOD_MS	100
#define R_DATA_MAX_TRY		5


#define REQ_WRITE_BYTES		0x00
#define REQ_WRITE_BITS		0x01
#define REQ_READ_BYTES		0x02
#define REQ_READ_BITS			0x03
#define REQ_WRITE_PAGE		0x05
#define REQ_REPLACE_ALG		0x06
#define REQ_REMOVE_ALG		0x07
#define REQ_STOP_ALG			0x08
#define REQ_WRITE_BIT			0x09
#define REQ_WRITE_RD			0x0A
#define REQ_REINIT_IO			0x0B
#define REQ_WRITE_EEPROM	0x0C 
#define REQ_WRITE_TWI_TAB	0x0D

#define IDLE						0
#define SYNC						1
#define DATA_RECIVED		2
#define MASTER_MODE			4
#define LAST_MASTER			8
#define HIGH_PRIORITY		16
#define TASK_PERFORM		32


#define F_NOT_TRANSIT		0x80
#define F_NO_ANSWER			0x40

#ifdef NPSYSTEM_FIRMWARE_API_CPP

#ifdef _MSC_VER
#pragma warning(disable : 4200)
#endif

#pragma pack(push, 1)

#endif

// Frame Header
	typedef struct _frame_h {
		uint8_t sl_addr;
		uint8_t m_addr;
		uint8_t fun_num;
		uint8_t len;  // (tagFrameHeader + payload + crc)
	} frame_h;

	// 
	typedef struct _req_write_bytes {
		uint8_t len;
		uint16_t addr;
		uint8_t data[];
	} req_write_bytes;
	
	typedef req_write_bytes ans_write_bytes;

	// 
	typedef struct _wr_bit_rec {
		uint16_t bit_addr;
		uint8_t bit_mask;
		uint8_t bit_val;
		uint8_t bit_status;
	} wr_bit_rec;

	typedef struct _req_write_bits {
		uint8_t nbits;
		wr_bit_rec data[];
	} req_write_bits;

	typedef req_write_bits ans_write_bits;
	typedef req_write_bytes ans_write_bytes;

	// 
	typedef struct _req_read_bytes {
		uint8_t nbytes;
		uint16_t addr_begin;
		uint16_t crc;
	} req_read_bytes;

	typedef struct {
		uint8_t nbytes;
		uint8_t data[];
	} ans_read_bytes;

	// 
	typedef req_read_bytes req_read_bits;
	typedef ans_read_bytes ans_read_bits;

#ifndef SPM_PAGESIZE
#ifndef NPSYSTEM_FIRMWARE_API_CPP
#error "SPM_PAGESIZE is not defined"
#endif
#define SPM_PAGESIZE 0
#endif

	// Write Page
	typedef struct _req_write_page {
		uint8_t page;
		uint8_t size;
		uint8_t data[];
	} req_write_page;

	typedef req_write_page req_write_rd;
	typedef req_write_page req_write_twi;

	// Replace Alg
	typedef struct _req_replace_alg {
		uint16_t addr_old;
		uint16_t addr_new;
		uint16_t crc;
	} req_replace_alg;

	// Remove Alg
	typedef struct _req_remove_alg {
		uint16_t addr;
		uint16_t crc;
	} req_remove_alg;

	// Stop Alg
	typedef struct _req_stop_alg {
		uint16_t addr;
		uint16_t crc;
	} req_stop_alg;

	typedef struct _ans_stop_alg {
		uint8_t result;
		uint16_t crc;
	} ans_stop_alg;

	// Write Bit
	typedef struct _req_write_bit {
		uint16_t bit_addr;
		uint8_t bit_n;
		uint8_t bit_val;
		uint16_t crc;
	} req_write_bit;
	typedef req_write_bit ans_write_bit;

	// Write EPPROM
	typedef struct _req_write_epprom {
		uint16_t addr;
		uint16_t len;
		uint8_t data[];
	} req_write_epprom;

	// Write TWI_TAB

	typedef struct _Frame {
		frame_h h;
		union
		{
#ifndef NPSYSTEM_FIRMWARE_API_CPP

#ifdef PC_LINK
			uint8_t data[sizeof(req_write_page) + 128 + 2];
#else
			uint8_t data[sizeof(req_write_page) + 128 + 2];
#endif

#endif
			req_write_bytes r_write_bytes;
			ans_write_bytes a_write_bytes;

			req_write_bits r_write_bits;
			ans_write_bits a_write_bits;

			req_read_bytes r_read_bytes;
			ans_read_bytes a_read_bytes;

			req_read_bits r_read_bits;
			ans_read_bits a_read_bits;

			// WRITE BIT WITHOUT STATUS
			req_write_bit r_write_bit;
			ans_write_bit a_write_bit;

			// NO ANSWER REQUESTS
			req_write_page r_wr_p;
			req_replace_alg r_rep_alg;
			req_remove_alg r_remove_alg;
			req_stop_alg r_stop_alg;
			ans_stop_alg a_stop_alg;

			req_write_rd r_write_rd;
			req_write_epprom r_write_epprom;
			req_write_twi r_write_twi;
		};
	} Frame;

	// Marker Values
#define M_REQ_CONTINUE						0
#define M_ARRAY_END								1
#define M_LAST_DATA_IN_REQUEST		2

	struct remote_data {
		union
		{
			struct {
				uint16_t remote_addr : 10;
				uint16_t is_bit : 1;
				uint16_t device_addr : 5;
				uint16_t local_addr : 10;
				uint16_t data_rw_type : 1;
				uint16_t bit_size : 3;
				uint16_t marker : 2;
			} val;
			uint32_t u32Val;
		};
		uint8_t unreachable_cnt;
	};

	typedef struct _RuntimeInfo {
		uint8_t result;
		uint8_t synclost;
		uint8_t e_slave_timeout;
		uint8_t rv_error_cnt;
		uint8_t rv_error_all_cnt;
	} RuntimeInfo;

	typedef struct _RuntimeInfoPCLink {
		RuntimeInfo com;
	} RuntimeInfoPCLink;

	typedef struct _RuntimeInfoController {
		RuntimeInfo com;
		union {
			uint8_t u8_time;
			uint16_t u16_time;
			uint32_t u32_time;
		};
		uint8_t cpu_load;
	} RuntimeInfoController;

#ifdef NPSYSTEM_FIRMWARE_API_CPP
#pragma pack(pop)

#ifdef _MSC_VER
#pragma warning(default : 4200)
#endif

#endif

#ifndef NPSYSTEM_FIRMWARE_API_CPP
	void net_init(void);
	void net_poll(void);
	extern uint16_t get_crc(void* ptr, uint8_t size);
	extern void init_io(void);
	extern void load_tt(void);
	extern void core_run(void);
	extern uint8_t ibbpc_ev;
	extern uint8_t status;
	register uint8_t ibbpc_ev asm("r2");
	register uint8_t status asm("r3");
#endif

#endif

#ifdef __ASSEMBLER__
#define TIME_OFFSET 5
#define r_status r3
#endif

#define MAX_FRAME_LEN sizeof(Frame)
#ifdef NPSYSTEM_FIRMWARE_API_CPP
}; // net end
#endif
#endif /* NET_H_ */


