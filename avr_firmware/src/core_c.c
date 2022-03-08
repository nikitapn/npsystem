// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "msvc.h"
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/atomic.h>
#include "../include/avr_firmware/net.h"
#include "../include/avr_firmware/twi.h"
#include "../include/avr_firmware/core.h"
#include "cfg/config.h"
#include <npsys/memtypes.h>

extern void append_alg_to_end(uint16_t task_addr);
extern void load_flash(const void* src, void* dest, uint8_t len);
extern uint8_t write_page(void);

extern Frame frame;
extern struct remote_data rdata[R_DATA_MAX];
extern RuntimeInfoController info;
extern const void _noinit_start;
extern const void FLASH_TT;
extern const void FLASH_RD;
extern const void FLASH_TWI;

void core_init(void) __attribute__((section(".text.core")));
uint16_t adc_task(void) __attribute__((section(".text.core")));
uint16_t twi_task(void) __attribute__((section(".text.core")));
static uint8_t store_sram_to_flash(void) __attribute__((section(".text.core")));
static uint8_t find_task_index(uint16_t addr) __attribute__((section(".text.core")));
uint8_t replace_alg(void) __attribute__((section(".text.core")));
uint8_t remove_alg(void) __attribute__((section(".text.core")));
uint8_t stop_alg(uint16_t algAddr) __attribute__((section(".text.core")));

tasktab_t tt;
eeprcfg_t eeprcfg __attribute__((section(".noinit")));

__attribute__((section(".eeprom.data")))
const eeprcfg_t epr_eeprcfg = {0};

uint8_t twi_ix;
uint8_t twi_table[TWI_TABLE_SIZE];

#define PAGE(x) ((x) / SPM_PAGESIZE)

uint32_t timer_cnt_unsafe;
Time internal_time;

void update_counter(void) {
	static uint8_t last;
	static uint16_t last_1s;

	if (last != *(uint8_t*)&timer_cnt_unsafe) {
		last = timer_cnt_unsafe;
		ATOMIC_BLOCK(ATOMIC_FORCEON) {
			info.u32_time = timer_cnt_unsafe;
		}
		
		if (internal_time.sec.quality == VQ_BAD) {
			last_1s = info.u16_time;
			return;
		}

		if (info.u16_time - last_1s >= 100) {
			last_1s = info.u16_time;
			if (++internal_time.sec.value == 60) {
				internal_time.sec.value = 0;
				if (++internal_time.min.value == 60) {
					internal_time.min.value = 0;
					if (++internal_time.hour.value == 24) {
						internal_time.hour.value = 0;
					}
				}
			}
		}
	}
}

static uint8_t store_sram_to_flash(void) {
	uint8_t user_tt_size = tt.size - SYS_TASK_NUM;

	frame.r_wr_p.page = PAGE((uint16_t)&FLASH_TT); // bad assembly code here
	frame.r_wr_p.size = 2 + user_tt_size * 2;
	frame.r_wr_p.data[0] = user_tt_size;
	frame.r_wr_p.data[1] = 0;

	uint16_t* task = (uint16_t*)(frame.r_wr_p.data + 2);
	
	for (uint8_t i = SYS_TASK_NUM; i < tt.size; ++i)
		task[i - SYS_TASK_NUM] = tt.t[i].addr;

	return write_page();
}

static uint8_t find_task_index(uint16_t addr) {
	for (uint8_t i = SYS_TASK_NUM; i < tt.size; ++i) {
		if (tt.t[i].addr == addr) return i;
	}
	return 0xff;
}

uint8_t replace_alg(void) {
	if (frame.r_rep_alg.addr_old == 0x0000) {
		append_alg_to_end(frame.r_rep_alg.addr_new);
	} else {
		int8_t index = find_task_index(frame.r_rep_alg.addr_old);
		if (index == 0xFF) return NET_ALGORTIHM_NOT_FOUND;
		
		tt.t[index].addr = frame.r_rep_alg.addr_new;
		tt.t[index].counter = 0;
	}
	return store_sram_to_flash();
}

uint8_t remove_alg(void) {
	uint8_t index = find_task_index(frame.r_remove_alg.addr);
	if (index == 0xff) return NET_ALGORTIHM_NOT_FOUND;

	tt.size = tt.size - 1;
	tt.cur_task = 0;
	for (uint8_t i = index; i < tt.size; ++i) {
		tt.t[i].addr = tt.t[i + 1].addr;
		tt.t[i].counter = tt.t[i + 1].counter;
	}
	return store_sram_to_flash();
}

uint8_t stop_alg(uint16_t algAddr) {
	uint8_t index = find_task_index(algAddr);
	if (index == 0xff) return NET_ALGORTIHM_NOT_FOUND;

	tt.cur_task = 0;
	tt.t[index].counter = 0xFFFF;
	
	return 0;
}

void core_init(void) {
	eeprom_read_block((void*)&_noinit_start, (void*)0, MEMORY_LEN + sizeof(eeprcfg_t));
	
	twi_init();
	load_flash(&FLASH_RD, &rdata, sizeof(rdata));
	load_flash(&FLASH_TWI, &twi_table, sizeof(twi_table));
	load_tt();
	TCCR0 = (1 << CS02) | (1 << CS00); // div 1024
	TIMSK |= (1 << TOIE0);
}

void reset(void) {
	wdt_disable();
	wdt_enable(WDTO_250MS);
	while (1);
}

uint16_t adc_value[ADC_MAX];

uint16_t adc_task(void) {
	static uint8_t index;

	switch(eeprcfg.adc_state) 
	{
		case ADC_OFF:
			break;
		case ADC_INIT:
			ADCSRA = (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2) | (1<<ADEN);
			index = 0;
		case ADC_SET_CHANNEL:
			if (index >= eeprcfg.adc_n)
				index = 0;
			ADMUX = eeprcfg.admux_value[index];
			ADCSRA |= (1 << ADSC);
			eeprcfg.adc_state = ADC_CONVERTING;
			break;
		case ADC_CONVERTING:
			if (ADCSRA & (1 << ADIF))  {
				ADCSRA |= (1 << ADIF);
				uint16_t value = ADCW;
				adc_value[eeprcfg.admux_value[index] & 0x07] = value;
				eeprcfg.adc_state = ADC_DELAY;
				index++;
			}
			break;
		case ADC_DELAY:
			eeprcfg.adc_state = ADC_SET_CHANNEL;
			break;
		case ADC_DISABLE:
			ADCSRA = 0;
			eeprcfg.adc_state = ADC_OFF;
			break;
	};
	
//	TIMSK &= ~(1 << TOIE0);
	static uint8_t cnt;
	info.cpu_load = info.u8_time - cnt;
	cnt = info.u8_time;
//	TIMSK |= (1 << TOIE0);

	LED_PORT ^= (1 << WORK_LED); 
	
	return ADC_TASK_DELAY;
}

uint16_t twi_task(void) {
	twitab_t* tab = (void*)twi_table;

	if (!tab->n || twi_result == TWI_BUSY) return I2C_TASK_DELAY;

	if (twi_result != TWI_FREE) {
		twi_req_t* cur = &tab->twi_req[twi_ix];
		cur->status = twi_result;

		if (cur->fn >= TWI_REQ_READ) {
			if (twi_result == TWI_SUCCESS) {
				uint8_t* r_buf = twi_buf + 3;
				memcpy(cur->data, r_buf, cur->len);
			} else {
				memset(cur->data, 0, cur->len);
			}
		}
	}
	
	if (++twi_ix == tab->n) twi_ix = 0;
	
	twi_req_t* next = &tab->twi_req[twi_ix];
	
	switch (next->fn) {
	case TWI_REQ_READ:
		twi_read(next);
		break;
	case TWI_REQ_READ_BLOCK:
		twi_read_bytes(next);
		break;
	case TWI_REQ_WRITE:
		twi_write(next);
		break;
	case TWI_REQ_WRITE_BLOCK:
		twi_write_bytes(next);
		break;
	default:
		twi_result = TWI_FREE;
		break;
	}

	return I2C_TASK_DELAY;
}

int main(void) {
	init_io();
	core_init();
	net_init();
	
	sei();
	core_run();
}

