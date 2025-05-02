#include "common.h"

int main()
{
  uint8_t a = 11;
  uint8_t b = 12;
  uint8_t c = a * b;

  *((uint8_t*)0x0060) = c;

  DEBUG_CMD_SIGNAL();
}
