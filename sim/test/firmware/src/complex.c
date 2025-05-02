#include "common.h"

float calc()
{
  float        a = 5.5, b = 2.2;
  int          x = 10, y = 3;
  unsigned int u = 7, v = 4;

  float result =
    (a * b) / (x + y) + ((float)(x * y) / (a - b)) - ((float)(u * v) / (x - y));
  result += ((float)(x / y) * (u + v)) - ((a / b) * (x - y)) +
            ((float)(u / v) * (y + x));
  return result;
}

int main()
{
  *((float*)0x0060) = calc();
  DEBUG_CMD_SIGNAL();
}
