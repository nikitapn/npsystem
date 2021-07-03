#include <avr/io.h>
#include <avr/interrupt.h>

#define loop_r		r23
#define spmcrval	r22
#define temp1		r24
#define temp2		r25

// | slave_addr | master_addr | fun | mess_len | page_num | page_size | data .... | crc_l | crc_h |

.equ page_num, frame + 4
.equ page_size, frame + 5
.equ page_data, frame + 6

.section .nrww, "ax", @progbits

.global write_page
write_page:
;Self programming part 
;Atmega8: pagesize - 32 words; flash - 4096 words or 128 pages;
;Atmega16: pagesize - 64 words; flash - 8192 words or 128 pages;
; page erase
	cli

	clr zl
	lds zh, page_num

#ifdef __AVR_ATmega8__
	lsr zh
	ror zl
	lsr zh
	ror zl
#endif

#ifdef __AVR_ATmega16__
	lsr zh
	ror zl
#endif

	ldi spmcrval, (1 << PGERS) | (1 << SPMEN)
	rcall Do_spm
; re-enable the RWW section
	ldi spmcrval, (1 << RWWSRE) | (1 << SPMEN)
	rcall Do_spm
; transfer data from RAM to Flash page buffer
	lds loop_r,page_size ;init loop variable
	ldi xl,lo8(page_data)
	ldi xh,hi8(page_data)
Wrloop:
	ld r0, x+
	ld r1, x+
	ldi spmcrval, (1 << SPMEN)
	rcall Do_spm
	adiw zl, 2
	subi loop_r, 2 ;use subi for PAGESIZEB<=256
	brne Wrloop
; execute page write
	lds temp1, page_size
	sub	 ZL, temp1	 ;restore pointer
	sbci ZH, 0 ;
	ldi spmcrval, (1 << PGWRT) | (1 << SPMEN)
	rcall Do_spm
; re-enable the RWW section
	ldi spmcrval, (1 << RWWSRE) | (1 << SPMEN)
	rcall Do_spm
; read back and check, optional
	lds loop_r,page_size		;init loop variable
	ldi xl,lo8(page_data)		;restore pointer
	ldi xh,hi8(page_data) 
Rdloop:
	lpm r0, Z+
	ld r1, x+
	cpse r0, r1
	rjmp Error_prog
	dec loop_r		;use subi for PAGESIZEB<=256
	brne Rdloop
; return to RWW section
; verify that RWW section is safe to read
Return:
	in temp1, _SFR_IO_ADDR(SPMCR)
	sbrs temp1, RWWSB ; If RWWSB is set, the RWW section is not
	; ready yet
	rjmp Success_prog
	; re-enable the RWW section
	ldi spmcrval, (1 << RWWSRE) | (1 << SPMEN)
	rcall Do_spm
	rjmp Return

Do_spm:
	; check for previous SPM complete
Wait_spm:
	in temp1, _SFR_IO_ADDR(SPMCR)
	sbrc temp1, SPMEN
	rjmp Wait_spm
	; input: spmcrval determines SPM action
	; disable interrupts if enabled, store status
;	in temp2, _SFR_IO_ADDR(SREG)
	;cli
	; check that no EEPROM write access is present
Wait_ee:
	sbic _SFR_IO_ADDR(EECR), EEWE
	rjmp Wait_ee
	; SPM timed sequence
	out _SFR_IO_ADDR(SPMCR), spmcrval
	spm
	; restore SREG (to enable interrupts if originally enabled)
;	out _SFR_IO_ADDR(SREG), temp2
;	clr r1
	ret

Success_prog:
	sei
	clr r1
	mov r24, r1
	ret
Error_prog:
	sei
	clr r1
	ldi r24, 0xff ; NET_WRITE_PAGE_FAIL
	ret