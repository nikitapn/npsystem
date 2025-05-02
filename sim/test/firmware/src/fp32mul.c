#include "common.h"

int main()
{
  volatile float a = 0.95f;
  volatile float b = 0.12f;

  *((float*)0x0060) = a * b;

  DEBUG_CMD_SIGNAL();
}

