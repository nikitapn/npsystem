#include "asmdef.h"
.section .text.libgcc.div, "ax", @progbits

/*******************************************************
       Division 8 / 8 => (result + remainder)
*******************************************************/
#define	r_rem	r25	/* remainder */
#define	r_arg1	r24	/* dividend, quotient */
#define	r_arg2	r22	/* divisor */
#define	r_cnt	r23	/* loop count */

DEFUN __udivmodqi4
	sub	r_rem,r_rem	; clear remainder and carry
	ldi	r_cnt,9		; init loop counter
	rjmp	__udivmodqi4_ep	; jump to entry point
__udivmodqi4_loop:
	rol	r_rem		; shift dividend into remainder
	cp	r_rem,r_arg2	; compare remainder & divisor
	brcs	__udivmodqi4_ep	; remainder <= divisor
	sub	r_rem,r_arg2	; restore remainder
__udivmodqi4_ep:
	rol	r_arg1		; shift dividend (with CARRY)
	dec	r_cnt		; decrement loop counter
	brne	__udivmodqi4_loop
	com	r_arg1		; complement result
				; because C flag was complemented in loop
	ret
ENDF __udivmodqi4

DEFUN __divmodqi4
        bst     r_arg1,7	; store sign of dividend
        mov     __tmp_reg__,r_arg1
        eor     __tmp_reg__,r_arg2; r0.7 is sign of result
        sbrc	r_arg1,7
	neg     r_arg1		; dividend negative : negate
        sbrc	r_arg2,7
	neg     r_arg2		; divisor negative : negate
	XCALL	__udivmodqi4	; do the unsigned div/mod
	brtc	__divmodqi4_1
	neg	r_rem		; correct remainder sign
__divmodqi4_1:
	sbrc	__tmp_reg__,7
	neg	r_arg1		; correct result sign
__divmodqi4_exit:
	ret
ENDF __divmodqi4


#undef r_rem
#undef r_arg1
#undef r_arg2
#undef r_cnt