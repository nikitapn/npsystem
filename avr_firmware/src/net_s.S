# Copyright (c) 2021 nikitapnn1@gmail.com
# This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <avr/io.h>
#include "../include/avr_firmware/net.h"

#define PC_LINK

#ifdef PC_LINK
.global	TIMER2_COMP_vect
TIMER2_COMP_vect:
	push r16
	in r16, _SFR_IO_ADDR(SREG)
	push r16
	inc r2
	pop r16
	out _SFR_IO_ADDR(SREG), r16
	pop r16
	reti
#else
//
.global	TIMER2_COMP_vect
TIMER2_COMP_vect:
	push r16
	in r16, _SFR_IO_ADDR(SREG)
	push r16
	
	inc r2

	bst r_status, 5
	brts save_context
net_continue:

	pop r16
	out _SFR_IO_ADDR(SREG), r16
	pop r16
	reti

save_context:
	push r0
	push r1
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	// 16
	push r17
	push r18
	push r19
	push r20
	push r21
	push r22
	push r23
	push r24
	push r25
	push r26
	push r27
	push r28
	push r29
	push r30
	push r31
	
	clt
	bld r_status, 5

	sei
	rcall net_poll
	cli

	set
	bld r_status, 5

	pop r0
	pop r1
	pop r4
	pop r5
	pop r6
	pop r7
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
	// 16
	pop r17
	pop r18
	pop r19
	pop r20
	pop r21
	pop r22
	pop r23
	pop r24
	pop r25
	pop r26
	pop r27
	pop r28
	pop r29
	pop r30
	pop r31

	rjmp net_continue
#endif // PC_LINK
