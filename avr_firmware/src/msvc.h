// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

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