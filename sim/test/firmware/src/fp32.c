#include "common.h"

int main()
{
  volatile float a = 0.95;
  volatile float b = 0.12;

  *((float*)0x0060) = a * b;

  DEBUG_CMD_SIGNAL();
}

