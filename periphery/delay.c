#include "delay.h"

// TODO: Make beautiful delays
void UTIL_Loop(unsigned int loop)
{
  while(loop--);	
}

void UTIL_WaitTimeInMs(unsigned int mck,
                       unsigned int time_ms)
{
  register unsigned int i = 0;
  i = (mck / 1000) * time_ms;
  i = i / 3;
  UTIL_Loop(i);
}

void UTIL_WaitTimeInUs(unsigned int mck,
                       unsigned int time_us)
{
  volatile unsigned int i = 0;
  i = (mck / 1000000) * time_us;
  i = i / 3;
  UTIL_Loop(i);
}
