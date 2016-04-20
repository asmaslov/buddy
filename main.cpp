#include "debugport.h"
#include "assert.h"
#include "trace.h"
//#include "twid.h"

#include "lcd.h"
//#include "usartd.h"
//#include "delay.h"

#include <stdio.h>
//#include <iostream>

void Delay (unsigned long a) { while (--a != 0); }

int main(void)
{   
  DebugPort *debug = new DebugPort();
  debug->printf("Debug port test string\n\r");

  printf("Trace level %d\n\r", TRACE_LEVEL); 
  printf("Message %d\n\r", MESSAGE); 

  TRACE_DEBUG("Main program start\n\r");
  TRACE_DEBUG("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
  TRACE_INFO("Firmware version 0.0.1 unstable\n\r");
  
  //cout << "This string is cout << inside IAR";
  
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
  while(1);
}
