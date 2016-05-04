#include "delay.h"
#include "board.h"

void delayLoop(unsigned int loop)
{
  while(loop--)
  {
    asm volatile
    (
      "nop\n\t"
      "nop\n\t"
      :::
    );
  }	
}

void delayMs(unsigned int time_ms)
{
  register unsigned int i = 0;
  i = (BOARD_MCK / 1000) * time_ms;
  i = i / 6;
  delayLoop(i);
}

void delayUs(unsigned int time_us)
{
  volatile unsigned int i = 0;
  i = (BOARD_MCK / 1000000) * time_us;
  i = i / 6;
  delayLoop(i);
}
