#include "debugport.h"
#include "trace.h"
#include "i2cd.h"

#include "pio.h"

#include "lcd.h"
#include "commander.h"
#include "usartd.h"
#include "parser.h"

//#include "delay.h"

#include <stdio.h>
#include <iostream>

// Slave address of chip
#define PCF_ADDRESS 0x39

#define MAIN_LOOP_DELAY 1000000

void Delay (unsigned long a) { while (--a != 0); }

int main(void)
{   
  TRACE_INFO("Firmware version 0.0.1 unstable\n\r");
  TRACE_DEBUG("Compiled: %s %s \n\r", __DATE__, __TIME__);
  TRACE_DEBUG("Main program start\n\r");
  static const Pin Buttons_pins[] = {
    PINS_PUSHBUTTONS,
  };
  PIO_Configure(Buttons_pins, PIO_LISTSIZE(Buttons_pins));
  bool sw1, sw2;
  static const Pin Joystick_pins[] = {
    PINS_JOYSTICK,
  };
  PIO_Configure(Joystick_pins, PIO_LISTSIZE(Joystick_pins));
  bool joyup, joydown, joyleft, joyright, joysw;
  unsigned long tick = 0;
  bool enable = true;

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

  Delay(1000);
  
  CommandVault cmdVault;
  Parser parser(&cmdVault);
  USARTDriver comport;
  comport.configure(USART0, 57600);
  comport.setParserFunc(parser.work);
   
  Delay(1000);
  comport.udmaprintf("USART test string");
  
  //Commander cmd(&cmdVault);
  //cmd.start();
    
  //I2CDriver i2c;
  //i2c.configureMaster();
   
  //unsigned char message[3] = {0, 1, 2}; 
  //i2c.write(PCF_ADDRESS, &message[0], 1);

  //initPWMD();
  
  TRACE_DEBUG("Main program end\n\r");
  // TODO: Make a nice standby mode instead of while(1)
  while(1)
  {
    if((tick++ > MAIN_LOOP_DELAY) && (enable))
    {
      sw1 = !PIO_Get(&Buttons_pins[PUSHBUTTON_BP1]);
      sw2 = !PIO_Get(&Buttons_pins[PUSHBUTTON_BP2]);
      joyup = !PIO_Get(&Joystick_pins[JOYSTICK_UP]);
      joydown = !PIO_Get(&Joystick_pins[JOYSTICK_DOWN]);
      joyleft = !PIO_Get(&Joystick_pins[JOYSTICK_LEFT]);
      joyright = !PIO_Get(&Joystick_pins[JOYSTICK_RIGHT]);
      joysw = !PIO_Get(&Joystick_pins[JOYSTICK_BUTTON]);
      TRACE_DEBUG("SW1 SW2 JSL JSR JSU JSD JSS\n\r");
      TRACE_DEBUG(" %c   %c   %c   %c   %c   %c   %c\n\r",
                  sw1 ? '+' : '-',
                  sw2 ? '+' : '-',
                  joyup ? '+' : '-',
                  joydown ? '+' : '-',
                  joyleft ? '+' : '-',
                  joyright ? '+' : '-',
                  joysw ? '+' : '-');
      // Start here
      


      if(sw1)
      {
        unsigned char dummy2 = 0;  
        //i2c.read(PCF_ADDRESS, &dummy2, 1);
        TRACE_DEBUG("I2C data %X\n\r", dummy2);
      }
       
      if(cmdVault.requests.buttonA)
      {
        TRACE_DEBUG("YES\n\r");
      }
      
      
      
      // End here
      tick = 0;   
    }
  }
}
