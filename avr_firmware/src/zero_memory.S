#include <avr/io.h>
.section .init3, "ax", @progbits

	ldi		xl,lo8(RAMSTART)	
	ldi		xh,hi8(RAMSTART)
zero_loop:
	st 		x+, r1				
	cpi		xh,hi8(RAMEND)		
	brne	zero_loop
	cpi		xl,lo8(RAMEND)		
	brne	zero_loop
