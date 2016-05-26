#include "smplmath.h"

unsigned int min(unsigned int a,
                 unsigned int b)
{
  if (a < b)
  {
    return a;
  }
  else
  {
    return b;
  }
}

unsigned int absv(signed int value)
{
  if (value < 0)
  {
    return -value;
  }
  else
  {
    return value;
  }
}

int sign(signed int value)
{
  return (value < 0) ? -1 : 1;
}

unsigned int power(unsigned int x,
                   unsigned int y)
{
  unsigned int result = 1;
  while (y > 0)
  {
    result *= x;
    y--;
  } 
  return result;
}
