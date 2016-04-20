#include "debugport.h"
//#include "twid.h"

#include "lcd.h"
//#include "usartd.h"
//#include "delay.h"

#include <stdio.h>
//#include <iostream>

void Delay (unsigned long a) { while (--a != 0); }

DebugPort *debug;

int main(void)
{   
  printf("This string is printf() inside IAR\n\r");
  printf("This string is printf() inside IAR\n\r");
  
  //cout << "This string is cout << inside IAR";


  debug = new DebugPort();
  debug->printf("This string is printf() to DBGU\n\r");
  debug->printf("This string is printf() to DBGU\n\r");
  
  
  printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
  
  //UTIL_WaitTimeInMs(BOARD_MCK, 1000);
  //UTIL_WaitTimeInUs(BOARD_MCK, 1000);
  
  LCD_init();
  Delay(1000);
  //UTIL_WaitTimeInMs(BOARD_MCK, 1000);

  // Set settings
  LCDSettings();
  
  Delay(1000);

  // Load bitmap
  LCDWrite130x130bmp();

  //InitUSART0();

  //write_str_USART0("Begin!");
  
  //initPWMD();
  
  //initTWID();
  
  // TODO: Make a nice standby mode instead of stupid while(1)

}
