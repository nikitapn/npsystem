# Copyright (c) 2021 nikitapnn1@gmail.com
# This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <avr/io.h>
#include "../include/avr_firmware/core.h"
#include "../include/avr_firmware/net.h"

// .equ TCNT_VALUE, 148 // 11.0592Mhz - 10000us
.equ TCNT_VALUE, 99 // 16Mhz - 10048us

.extern net_poll

.section .text.core, "ax", @progbits

; (const void* src, void* dest, uint8_t len);
.global load_flash
load_flash:
	mov zl, r24
	mov zh, r25
	mov xl, r22
	mov xh, r23
copy_flesh:
	lpm
	st x+, r0
	adiw z, 1
	dec r20
	brne copy_flesh
	ret

.global load_tt
load_tt:
/*
	|num of task|current task|addr0_l|addr0_h| ............ |addrN_l|addrN_h|
*/
	ldi		r16, SYS_TASK_NUM // adc_task + twi_task
	sts		tt, r16
	// adc_task
	ldi		r16, pm_lo8(adc_task)
	sts		tt+2, r16

	ldi		r16, pm_hi8(adc_task)
	sts		tt+3, r16

	// twi_task
	ldi		r16, pm_lo8(twi_task)
	sts		tt+6, r16

	ldi		r16, pm_hi8(twi_task)
	sts		tt+7, r16

	ldi		zl, lo8(FLASH_TT)
	ldi		zh, hi8(FLASH_TT)
	
	LPM		R17, Z

	TST		R17
	BREQ	no_tasks	;

	ADIW	z, 2		; num of tasks second byte not usefull
task_copy:
	LPM		R24,Z+		;addr_l
	LPM		R25,Z+		;addr_h
	RCALL	append_alg_to_end	
	DEC		R17
	BRNE	task_copy
no_tasks:
	RET

;void append_alg_to_end(uint16_t task_addr);
.global append_alg_to_end
append_alg_to_end:
/*
	|num of task|current task|addr0_l|addr0_h|timer0_l|timer0_h| ............ |addrN_l|addrN_h|timerN_l|timerN_h|
*/
	LDI		xl,lo8(tt)
	LDI		xh,hi8(tt)
	; increase counter
	LD		R18, x
	
	INC		R18
	ST		x, R18
	DEC		R18

	ADIW	x, 2

	LDI		R19, 4
	MUL		R18, R19

	ADD		XL, R0
	ADC		XH, R1
	
	eor		R1, R1

	ST		x+, R24	; addr_l
	ST		x+, R25	; addr_h
	ST		x+, R1	; timer_h
	ST		x+, R1	; timer_l
	
	RET

.global core_run
core_run:
	/*
	|num of task|current task|addr0_h|addr0_l|timer0_h|timer0_l| ............ |addrN_h|addrN_l|timerN_h|timerN_l|
	*/
	rcall	net_poll
	rcall	update_counter

	bst r_status, 4 ; HIGH_PRIORITY
	brts core_run

	LDI		xl, lo8(tt)
	LDI		xh, hi8(tt)

	LD		r16, x+
	LD		r17, x+

//	and		r16, r61
//	breq	core_run;

	ldi		r18, 4
	mul		r17, r18

	add		xl, r0
	adc		xh, r1

	eor		r1, r1

	LD		zl, x+
	LD		zh, x+

	cli			
	LD		R24, x+
	LD		R25, x+
	sei

	TST		R24
	BRNE	next_task
	TST		R25
	BRNE	next_task
	
	push	xl
	push	xh

	set
	bld		r_status, 5

	ICALL ; the fuction might not save R2-R17, R28, R29
	
	clt 
	bld		r_status,5

	pop		xh
	pop		xl

	; store return value (R24:R25) for current task 
	cli
	ST		-x, R25
	ST		-x, R24
	sei	

	lds		r16, tt
	lds		r17, tt + 1

next_task:
	inc		r17
	cpse	r16, r17
	rjmp	update_cur_task		
	clr		r17 // to begin
update_cur_task:
	sts		tt + 1, r17
	rjmp core_run

; main timer one tick - 10ms
.global TIMER0_OVF_vect
TIMER0_OVF_vect:
	push	r23
	in		r23,_SFR_IO_ADDR(SREG)
	push	r23
	push	r24
	push	r25
	push	r26
	push	r27

	/*
	|num of task|current task|addr0_h|addr0_l|timer0_h|timer0_l| ............ |addrN_h|addrN_l|timerN_h|timerN_l|
	*/
	LDI		XL,lo8(tt)
	LDI		XH,hi8(tt)

	LD		r23, x
	TST		r23
	breq	_int_main_exit

	ADIW	x, 4
int_main_loop:
	LD		R24,x+
	LD		R25,x
	
	TST		R25
	BRNE	_sub_1
	TST		R24
	BRNE	_sub_1
	ADIW	x,3					
	RJMP	_int_main_next_task
_sub_1:
	SBIW	R24,1
	ST		x,R25
	ST		-x,R24
	ADIW	x,4
_int_main_next_task:
	DEC		r23
	BRNE	int_main_loop

_int_main_exit:
	ldi R24, TCNT_VALUE
	out _SFR_IO_ADDR(TCNT0), R24

;for delay blocks
	lds R24, timer_cnt_unsafe + 0
	lds R25, timer_cnt_unsafe + 1
	lds R26, timer_cnt_unsafe + 2
	lds R27, timer_cnt_unsafe + 3

	adiw r24, 1
	adc r26, r23
	adc r27, r23
	
	sts timer_cnt_unsafe + 0, R24
	sts timer_cnt_unsafe + 1, R25
	sts timer_cnt_unsafe + 2, R26
	sts timer_cnt_unsafe + 3, R27
; R_DATA_PERIOD
	lds r24, r_data_cnt
	tst r24
	breq clear_stack
	dec r24
	sts r_data_cnt, r24
clear_stack:
	pop r27
	pop r26
	pop r25
	pop r24
	pop r23
	out _SFR_IO_ADDR(SREG),r23
	pop r23
	reti