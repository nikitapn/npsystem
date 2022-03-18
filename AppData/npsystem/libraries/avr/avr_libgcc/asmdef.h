/*  -*- Mode: Asm -*-  */
/* Copyright (C) 1998-2017 Free Software Foundation, Inc.
   Contributed by Denis Chertykov <chertykov@gmail.com>

This file is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.

You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */
#define L_udivmodhi4
#if defined (__AVR_TINY__)
#define __zero_reg__ r17
#define __tmp_reg__ r16
#else
#define __zero_reg__ r1
#define __tmp_reg__ r0
#endif
#define __SREG__ 0x3f
#if defined (__AVR_HAVE_SPH__)
#define __SP_H__ 0x3e
#endif
#define __SP_L__ 0x3d
#define __RAMPZ__ 0x3B
#define __EIND__  0x3C

/* Most of the functions here are called directly from avr.md
   patterns, instead of using the standard libcall mechanisms.
   This can make better code because GCC knows exactly which
   of the call-used registers (not all of them) are clobbered.  */

/* FIXME:  At present, there is no SORT directive in the linker
           script so that we must not assume that different modules
           in the same input section like .libgcc.text.mul will be
           located close together.  Therefore, we cannot use
           RCALL/RJMP to call a function like __udivmodhi4 from
           __divmodhi4 and have to use lengthy XCALL/XJMP even
           though they are in the same input section and all same
           input sections together are small enough to reach every
           location with a RCALL/RJMP instruction.  */

#if defined (__AVR_HAVE_EIJMP_EICALL__) && !defined (__AVR_HAVE_ELPMX__)
#error device not supported
#endif

	.macro	mov_l  r_dest, r_src
#if defined (__AVR_HAVE_MOVW__)
	movw	\r_dest, \r_src
#else
	mov	\r_dest, \r_src
#endif
	.endm

	.macro	mov_h  r_dest, r_src
#if defined (__AVR_HAVE_MOVW__)
	; empty
#else
	mov	\r_dest, \r_src
#endif
	.endm

.macro	wmov  r_dest, r_src
#if defined (__AVR_HAVE_MOVW__)
    movw \r_dest,   \r_src
#else
    mov \r_dest,    \r_src
    mov \r_dest+1,  \r_src+1
#endif
.endm

#if defined (__AVR_HAVE_JMP_CALL__)
#define XCALL call
#define XJMP  jmp
#else
#define XCALL rcall
#define XJMP  rjmp
#endif

#if defined (__AVR_HAVE_EIJMP_EICALL__)
#define XICALL eicall
#define XIJMP  eijmp
#else
#define XICALL icall
#define XIJMP  ijmp
#endif

;; Prologue stuff

.macro do_prologue_saves n_pushed n_frame=0
    ldi r26, lo8(\n_frame)
    ldi r27, hi8(\n_frame)
    ldi r30, lo8(gs(.L_prologue_saves.\@))
    ldi r31, hi8(gs(.L_prologue_saves.\@))
    XJMP __prologue_saves__ + ((18 - (\n_pushed)) * 2)
.L_prologue_saves.\@:
.endm

;; Epilogue stuff

.macro do_epilogue_restores n_pushed n_frame=0
    in      r28, __SP_L__
#ifdef __AVR_HAVE_SPH__
    in      r29, __SP_H__
.if \n_frame > 63
    subi    r28, lo8(-\n_frame)
    sbci    r29, hi8(-\n_frame)
.elseif \n_frame > 0
    adiw    r28, \n_frame
.endif
#else
    clr     r29
.if \n_frame > 0
    subi    r28, lo8(-\n_frame)
.endif
#endif /* HAVE SPH */
    ldi     r30, \n_pushed
    XJMP __epilogue_restores__ + ((18 - (\n_pushed)) * 2)
.endm

;; Support function entry and exit for convenience

.macro wsubi r_arg1, i_arg2
#if defined (__AVR_TINY__)
    subi \r_arg1,   lo8(\i_arg2)
    sbci \r_arg1+1, hi8(\i_arg2)
#else
    sbiw \r_arg1, \i_arg2
#endif
.endm

.macro waddi r_arg1, i_arg2
#if defined (__AVR_TINY__)
    subi \r_arg1,   lo8(-\i_arg2)
    sbci \r_arg1+1, hi8(-\i_arg2)
#else
    adiw \r_arg1, \i_arg2
#endif
.endm

.macro DEFUN name
.global \name
.func \name
\name:
.endm

.macro ENDF name
.size \name, .-\name
.endfunc
.endm

.macro FALIAS name
.global \name
.func \name
\name:
.size \name, .-\name
.endfunc
.endm

;; Skip next instruction, typically a jump target
#if defined(__AVR_TINY__)
#define skip cpse 0,0
#else
#define skip cpse 16,16
#endif

;; Negate a 2-byte value held in consecutive registers
.macro NEG2  reg
    com     \reg+1
    neg     \reg
    sbci    \reg+1, -1
.endm

;; Negate a 4-byte value held in consecutive registers
;; Sets the V flag for signed overflow tests if REG >= 16
.macro NEG4  reg
    com     \reg+3
    com     \reg+2
    com     \reg+1
.if \reg >= 16
    neg     \reg
    sbci    \reg+1, -1
    sbci    \reg+2, -1
    sbci    \reg+3, -1
.else
    com     \reg
    adc     \reg,   __zero_reg__
    adc     \reg+1, __zero_reg__
    adc     \reg+2, __zero_reg__
    adc     \reg+3, __zero_reg__
.endif
.endm