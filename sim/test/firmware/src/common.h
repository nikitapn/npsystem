#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)

#define DEBUG_CMD_SIGNAL() \
  asm (".long " TOSTRING(0x0080))

#define RESULT8(x) \
  *(uint8_t*)(0x0060) = *(uint8_t*)&x

#define RESULT16(x) \
  *(uint16_t*)(0x0060) = *(uint16_t*)&x

#define RESULT32(x) \
  *(uint32_t*)(0x0060) = *(uint32_t*)&x

#endif  // COMMON_H_
