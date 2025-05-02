#include "common.h"

int main()
{
  uint8_t a = 10;
  uint8_t b = 20;
  uint8_t c = a + b;

  *((uint8_t*)0x0060) = c;

  DEBUG_CMD_SIGNAL();
}
