#include "common.h"

int main()
{
  volatile int a = 30;
  volatile int b = 240;
  volatile int c = a + b;

  RESULT16(c);
  DEBUG_CMD_SIGNAL();
}
