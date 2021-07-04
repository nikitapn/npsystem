// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "timers.h"

#if defined(__AVR_ATmega8__)
#	define TMR_CS								((1 << CS01) | (1 << CS00)) // 64
#	define TMR_TCNT_8000000			131
#	define TMR_TCNT_11059200		83
#	define TMR_TCNT_16000000		6
#else
#	error "no definition for MCU available in chipdef.h"
#endif

#if (F_CPU == 8000000UL)
#	define TMR_TCNT TMR_TCNT_8000000
#	define TMR_CLOCK TMR_CS
#elif (F_CPU == 11059200UL)
#	define TMR_TCNT TMR_TCNT_11059200
#	define TMR_CLOCK TMR_CS
#elif (F_CPU == 16000000)
#	define TMR_TCNT TMR_TCNT_16000000
#	define TMR_CLOCK TMR_CS
#else 
#	error "unknown frequency"
#endif

static uint16_t timer_cnt_unsafe;
static uint16_t timer_cnt;
static uint16_t timers[TIMERS_MAX];

ISR(TIMER0_OVF_vect) {
	timer_cnt_unsafe++;
	TCNT0 = TMR_TCNT;
}

void timers_init(void) {
	TIMSK |= (1 << TOIE0);
	TCNT0 = TMR_TCNT;
	TCCR0 = TMR_CLOCK;
}

void timers_update(void) {
	static uint8_t last;
	if (last != *(uint8_t*)&timer_cnt_unsafe)  {
		last = timer_cnt_unsafe;
		ATOMIC_BLOCK(ATOMIC_FORCEON) {
			timer_cnt = timer_cnt_unsafe;
		}
	}
}

void timer_reset(uint8_t timer_id) {
	timers[timer_id] = timer_cnt;
}

uint16_t timer_get_count(uint8_t timer_id) {	
	return timer_cnt - timers[timer_id];
}

uint8_t timer_expired(uint8_t timer_id, uint16_t time) {
	uint8_t expired = (timer_get_count(timer_id) > time);
	if (expired) timer_reset(timer_id);
	return expired;
}
