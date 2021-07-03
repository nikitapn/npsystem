#ifndef __MSVC_H__
#define __MSVC_H__

#ifdef MSVC
	#define asm 
	#define asm(x) 
	#define __asm__ 
	#define __volatile__(x) 
	#define register 
	#define __AVR_ATmega16__
	#define __attribute__(x) 
	#define __inline__ 
	#define __extension__ 
	#define MY_ADDRESS 0
#endif // MSVC


#endif // __MSVC_H__