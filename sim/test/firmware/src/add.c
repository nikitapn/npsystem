#include "common.h"

int main()
{
  uint8_t a = 10;
  uint8_t b = 20;
  uint8_t c = a + b;

  RESULT8(c);
  DEBUG_CMD_SIGNAL();
}
