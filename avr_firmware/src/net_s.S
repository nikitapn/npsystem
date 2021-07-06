# Copyright (c) 2021 nikitapnn1@gmail.com
# This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <avr/io.h>
#include "../include/avr_firmware/net.h"

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