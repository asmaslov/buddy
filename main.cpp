#include "debugport.h"
#include "trace.h"
#include "i2cd.h"
#include "adcd.h"
#include "pwmcd.h"

#include "pio.h"

#include "lcd.h"
#include "commander.h"
#include "usartd.h"
#include "parser.h"

#include "delay.h"

#include <stdio.h>
#include <iostream>

// Slave address of chip
#define PCF_ADDRESS 0x39

#define MAIN_LOOP_DELAY 10000

int main(void)
{   
  TRACE_INFO("Firmware version 0.0.1 unstable\n\r");
  TRACE_DEBUG("Compiled: %s %s \n\r", __DATE__, __TIME__);
  TRACE_DEBUG("Main program start\n\r");
  static const Pin Buttons_pins[] = { PINS_PUSHBUTTONS };
  PIO_Configure(Buttons_pins, PIO_LISTSIZE(Buttons_pins));
  bool sw1, sw2;
  static const Pin Joystick_pins[] = { PINS_JOYSTICK };
  PIO_Configure(Joystick_pins, PIO_LISTSIZE(Joystick_pins));
  bool joyup, joydown, joyleft, joyright, joysw;
  bool enable = true;

  delayMs(100);
  LCD_init();
  delayUs(100);
  LCDSettings(); 
  delayUs(100);
  LCDWrite130x130bmp();
  delayUs(100);
  
  CommandVault cmdVault;
  Parser parser(&cmdVault);
  USARTDriver comport;
  delayMs(100);
  comport.configure(USART0, 57600);
  delayMs(100);
  comport.setParserFunc(parser.work);
   
  delayMs(100);
  comport.udmaprintf("USART test string");
  
  //Commander cmd(&cmdVault);
  //cmd.start();
    
  I2CDriver i2c;
  delayMs(100);
  i2c.configureMaster();
  delayMs(100);
   
  ADCDriver adc;
  delayMs(100);
  
  unsigned char message[2] = {0xFE, 0xFD};
  i2c.setAddress(PCF_ADDRESS);
  
  PWMCDriver pwm;
  pwm.init();
  
  int ppin = 0;
  static const Pin Test_pin = {BIT6, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT};
  PIO_Configure(&Test_pin, 1);
  
  
  TRACE_DEBUG("Main program end\n\r");
  // TODO: Make a nice standby mode instead of while(1)
  while(1)
  {
    delayMs(46);

    if(ppin == 1)
    {
      ppin = 0;
      PIO_Clear(&Test_pin);
    }
    else
    {
      ppin = 1;
      PIO_Set(&Test_pin);
    }
    
    if(enable)
    {
      sw1 = !PIO_Get(&Buttons_pins[PUSHBUTTON_BP1]);
      sw2 = !PIO_Get(&Buttons_pins[PUSHBUTTON_BP2]);
      joyup = !PIO_Get(&Joystick_pins[JOYSTICK_UP]);
      joydown = !PIO_Get(&Joystick_pins[JOYSTICK_DOWN]);
      joyleft = !PIO_Get(&Joystick_pins[JOYSTICK_LEFT]);
      joyright = !PIO_Get(&Joystick_pins[JOYSTICK_RIGHT]);
      joysw = !PIO_Get(&Joystick_pins[JOYSTICK_BUTTON]);
      adc.convert();
      TRACE_DEBUG("SW1 SW2 JSU JSD JSL JSR JSS\n\r");
      TRACE_DEBUG(" %c   %c   %c   %c   %c   %c   %c\n\r",
                  sw1 ? '+' : '-',
                  sw2 ? '+' : '-',
                  joyup ? '+' : '-',
                  joydown ? '+' : '-',
                  joyleft ? '+' : '-',
                  joyright ? '+' : '-',
                  joysw ? '+' : '-');
      TRACE_DEBUG("TEMP TRIM MIC\n\r");
      TRACE_DEBUG(" %d   %d   %d\n\r",
                  adc.getTemp(),
                  adc.getTrim() * 100 / ADC_VREF,
                  adc.getMicIn());
      // Start here
    
      if(sw1)
      {
        i2c.write(&message[0], 1);
        TRACE_DEBUG("I2C data write %X\n\r", message[0]);
      }
      if(sw2)
      {
        i2c.write(&message[1], 1);
        TRACE_DEBUG("I2C data write %X\n\r", message[1]);
      }
      if(joysw)
      {
        unsigned char dummy2 = 0;  
        i2c.read(&dummy2, 1);
        TRACE_DEBUG("I2C data read %X\n\r", dummy2);
      }
       
      if(cmdVault.requests.buttonA)
      {
        TRACE_DEBUG("Button A\n\r");
      }
      
      if(cmdVault.requests.buttonB)
      {
        TRACE_DEBUG("Button B\n\r");
      }
      
      // End here
    }
  }
}
