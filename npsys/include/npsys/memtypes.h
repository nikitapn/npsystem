#ifndef __MEMTYPES_H__
#define __MEMTYPES_H__

#define VQ_BAD			((uint8_t)0x00) 
#define VQ_GOOD			((uint8_t)0x01) 
#define VQ_UNKNOWN	((uint8_t)0x02) 


#define ALPHA_TRUE	((uint8_t)0xFF)
#define ALPHA_FALSE	((uint8_t)0x00)

#include <stdint.h>

#ifdef __cplusplus
#	pragma pack(push, 1)
#endif

typedef struct {
	uint8_t value;
} bit;

typedef struct {
	uint8_t value;
	uint8_t quality;
} Q_bit;

typedef struct {
	uint8_t value;
} u8;

typedef struct {
	uint8_t value;
	uint8_t quality;
} Q_u8;

typedef struct {
	int8_t value;
} i8;

typedef struct {
	int8_t value;
	uint8_t quality;
} Q_i8;

typedef struct {
	uint16_t value;
} u16;

typedef struct {
	uint16_t value;
	uint8_t quality;
} Q_u16;

typedef struct {
	int16_t value;
} i16;

typedef struct {
	int16_t value;
	uint8_t quality;
} Q_i16;

typedef struct {
	uint32_t value;
} u32;

typedef struct {
	uint32_t value;
	uint8_t quality;
} Q_u32;

typedef struct {
	int32_t value;
} i32;

typedef struct {
	int32_t value;
	uint8_t quality;
} Q_i32;

typedef struct {
	float value;
} flt;

typedef struct {
	float value;
	uint8_t quality;
} Q_flt;

#ifdef __cplusplus
namespace alpha {
#endif

typedef struct {
	Q_u8 sec;
	Q_u8 min;
	Q_u8 hour;
	Q_u8 wday;
	Q_u8 mday;
	Q_u8 mon;
	Q_u8 year;
} Time;

#ifdef __cplusplus
} // namespace alpha
#	pragma pack(pop)
#endif

#endif // !__MEMTYPES_H__
