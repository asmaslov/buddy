#include "debugport.h"
#include "trace.h"
#include "i2cd.h"

#include "lcd.h"
//#include "usartd.h"
//#include "delay.h"

#include <stdio.h>
#include <iostream>

#include <queue>

#define I2C_FREQ_HZ 5000


// Slave address of chip
#define PCF_ADDRESS 0x39


typedef struct _I2CNod {
  unsigned int id;
  queue<char> wrtiteBuffer;
  queue<char> readBuffer;
} I2CNod;

void Delay (unsigned long a) { while (--a != 0); }

int main(void)
{   
  TRACE_INFO("Firmware version 0.0.1 unstable\n\r");
  TRACE_DEBUG("Compiled: %s %s \n\r", __DATE__, __TIME__);
  TRACE_DEBUG("Main program start\n\r");
  
  //DebugPort *debug = new DebugPort();
  //debug->dbgprintf("Debug port test string\n\r");
  
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



    
  I2CDriver *i2c = new I2CDriver();

  
  i2c->configureMaster(I2C_FREQ_HZ);
  

  
  unsigned char message[3] = {0, 1, 2}; 

  

  //i2c->write(PCF_ADDRESS, message, 1);
  i2c->write(PCF_ADDRESS, &message[0], 1); 
  //i2c->writeNow(PCF_ADDRESS, &message[1], 1); 

  //InitUSART0();
 
  //write_str_USART0("Begin!");
  
  //initPWMD();
  
  //initTWID();
  
  // TODO: Make a nice standby mode instead of stupid while(1)
  TRACE_DEBUG("Main program end\n\r");
  while(1);
}
