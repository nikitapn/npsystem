// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#ifndef __MEMTYPES_H__
#define __MEMTYPES_H__

#define VQ_BAD			((uint8_t)0x00) 
#define VQ_GOOD			((uint8_t)0x01) 
#define VQ_UNKNOWN	((uint8_t)0x02) 


#define NPSYSTEM_TRUE	  ((uint8_t)0xFF)
#define NPSYSTEM_FALSE	((uint8_t)0x00)

#include <stdint.h>

#ifdef __cplusplus
namespace npsystem::types {
#	pragma pack(push, 1)
#endif

typedef struct {
	uint8_t value;
} BIT;

typedef struct {
	uint8_t value;
	uint8_t quality;
} QBIT;

typedef struct {
	uint8_t value;
} U8;

typedef struct {
	uint8_t value;
	uint8_t quality;
} QU8;

typedef struct {
	int8_t value;
} I8;

typedef struct {
	int8_t value;
	uint8_t quality;
} QI8;

typedef struct {
	uint16_t value;
} U16;

typedef struct {
	uint16_t value;
	uint8_t quality;
} QU16;

typedef struct {
	int16_t value;
} I16;

typedef struct {
	int16_t value;
	uint8_t quality;
} QI16;

typedef struct {
	uint32_t value;
} U32;

typedef struct {
	uint32_t value;
	uint8_t quality;
} QU32;

typedef struct {
	int32_t value;
} I32;

typedef struct {
	int32_t value;
	uint8_t quality;
} QI32;

typedef struct {
	float value;
} F32;

typedef struct {
	float value;
	uint8_t quality;
} QF32;

typedef struct {
	QU8 sec;
	QU8 min;
	QU8 hour;
	QU8 wday;
	QU8 mday;
	QU8 mon;
	QU8 year;
} Time;

#ifdef __cplusplus
} // namespace npsystem
#	pragma pack(pop)
#endif

#endif // !__MEMTYPES_H__
