#include "debugport.h"
#include "trace.h"
#include "i2cd.h"
#include "adcd.h"
#include "pwmcd.h"

#include "pio.h"

#include "tc.h"
#include "aic.h"

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

static int ppin = 0;
static const Pin Test_pin = {BIT6, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT};
static const Pin Clock_pin = {BIT22, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT};

#define STEP_MAX 500
#define STEP_MIN 10

volatile unsigned int tcrc = 100;
volatile unsigned int step = STEP_MIN;


volatile bool go_right = true;
volatile bool go_left = false;
  


void ISR_Tc0(void)
{
    // Clear status bit to acknowledge interrupt
    unsigned int dummy;
    dummy = AT91C_BASE_TC0->TC_SR;
    dummy = dummy;

    // Do
       
    AT91C_BASE_TC0->TC_RC = tcrc;

    TC_Start(AT91C_BASE_TC0);
       
    if(ppin == 1)
    {
      ppin = 0;
      PIO_Clear(&Test_pin);
      PIO_Clear(&Clock_pin);
    }
    else
    {
      ppin = 1;
      PIO_Set(&Test_pin);
      PIO_Set(&Clock_pin);
    }
    

    if(go_right)
    {
      step++;
      if(step > STEP_MAX)
      {
        go_right = false;
        go_left = true;
      }
    }
    else if(go_left)
    {
      step--;
      if(step < STEP_MIN)
      {
        go_right = true;
        go_left = false;
      }
    }
  
}


void ConfigureTc(void)
{
    unsigned int div;
    unsigned int tcclks;

    // Enable peripheral clock
    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_TC0;

    // Configure TC for a 4Hz frequency and trigger on RC compare
    TC_FindMckDivisor(100, BOARD_MCK, &div, &tcclks);
    TC_Configure(AT91C_BASE_TC0, tcclks | AT91C_TC_CPCTRG);
    AT91C_BASE_TC0->TC_RC = (BOARD_MCK / div) / 100; // timerFreq / desiredFreq

    // Configure and enable interrupt on RC compare
    AIC_ConfigureIT(AT91C_ID_TC0, AT91C_AIC_PRIOR_LOWEST, ISR_Tc0);
    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;
    AIC_EnableIT(AT91C_ID_TC0);

    TC_Start(AT91C_BASE_TC0);
}

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
  adc.start();
  
  unsigned char message[2] = {0xFF, 0xFC};
  i2c.setAddress(PCF_ADDRESS);
  
  PWMCDriver pwm;
  pwm.init();

  PIO_Configure(&Test_pin, 1);
  PIO_Configure(&Clock_pin, 1);
  ConfigureTc();
   
  TRACE_DEBUG("Main program end\n\r");
  // TODO: Make a nice standby mode instead of while(1)
  unsigned long mld = 0;
  while(1)
  {
    mld++;
    if((mld > MAIN_LOOP_DELAY) && enable)
    {
      mld = 0;
      sw1 = !PIO_Get(&Buttons_pins[PUSHBUTTON_BP1]);
      sw2 = !PIO_Get(&Buttons_pins[PUSHBUTTON_BP2]);
      joyup = !PIO_Get(&Joystick_pins[JOYSTICK_UP]);
      joydown = !PIO_Get(&Joystick_pins[JOYSTICK_DOWN]);
      joyleft = !PIO_Get(&Joystick_pins[JOYSTICK_LEFT]);
      joyright = !PIO_Get(&Joystick_pins[JOYSTICK_RIGHT]);
      joysw = !PIO_Get(&Joystick_pins[JOYSTICK_BUTTON]);
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
      
      unsigned int temp = 46875 / (adc.getTrim() * 1000 / ADC_VREF);
      tcrc = temp;
      
      if(go_right)
      {
        message[0] &=~(1 << 2);
        message[1] &=~(1 << 2);
      }
      else if(go_left)
      {
        message[0] |= (1 << 2);
        message[1] |= (1 << 2);
      };
      
      
      if(sw1)
      {
        i2c.write(&message[0], 1);
        TRACE_DEBUG("I2C data write %X\n\r", message[0]);
      }
      else if(sw2)
      {
        i2c.write(&message[1], 1);
        TRACE_DEBUG("I2C data write %X\n\r", message[1]);
      }
      else if(joysw)
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
