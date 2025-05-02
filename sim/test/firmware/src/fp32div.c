#include "common.h"

int main()
{
  volatile float a = 5.0f;
  volatile float b = 2.0f;

  *((float*)0x0060) = a / b;

  DEBUG_CMD_SIGNAL();
}
