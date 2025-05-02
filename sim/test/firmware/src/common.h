#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)

#define DEBUG_CMD_SIGNAL() \
  asm (".long " TOSTRING(0x0080))

#endif  // COMMON_H_
