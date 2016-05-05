#include "debugport.h"
#include "trace.h"
#include "adcd.h"
#include "pwmcd.h"

#include "pio.h"

#include "tc.h"
#include "aic.h"

#include "lcd.h"
#include "commander.h"


#include "delay.h"

#include <stdio.h>
#include <iostream>

#define MAIN_LOOP_DELAY 1000000

static int ppin = 0;
static const Pin Test_pin = {BIT6, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT};
static const Pin Clock_pin = {BIT22, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT};

#define STEP_MAX 1000
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
    AT91C_BASE_TC0->TC_RC = (BOARD_MCK / div) / 1000; // timerFreq / desiredFreq

    // Configure and enable interrupt on RC compare
    AIC_ConfigureIT(AT91C_ID_TC0, AT91C_AIC_PRIOR_LOWEST, ISR_Tc0);
    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;
    AIC_EnableIT(AT91C_ID_TC0);

    TC_Start(AT91C_BASE_TC0);
}

/*void sw1ISR_Bp1(void)
{
    static unsigned int lastPress = 0;

    // Check if the button has been pressed
    if (!PIO_Get(&pinPB1)) {

        // Simple debounce method: limit push frequency to 1/DEBOUNCE_TIME
        // (i.e. at least DEBOUNCE_TIME ms between each push)
        if ((timestamp - lastPress) > DEBOUNCE_TIME) {

            lastPress = timestamp;

            // Toggle LED state
            pLedStates[0] = !pLedStates[0];
            if (!pLedStates[0]) {

                LED_Clear(0);
            }
        }
    }
}

//------------------------------------------------------------------------------
/// Interrupt handler for pushbutton\#2. Starts or stops LED\#2 and TC0.
//------------------------------------------------------------------------------
void ISR_Bp2(void)
{
    static unsigned int lastPress = 0;

    // Check if the button has been pressed
    if (!PIO_Get(&pinPB2)) {

        // Simple debounce method: limit push frequency to 1/DEBOUNCE_TIME
        // (i.e. at least DEBOUNCE_TIME ms between each push)
        if ((timestamp - lastPress) > DEBOUNCE_TIME) {

            lastPress = timestamp;

            // Disable LED#2 and TC0 if there were enabled
            if (pLedStates[1]) {

                pLedStates[1] = 0;
                LED_Clear(1);
                AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
            }   
            // Enable LED#2 and TC0 if there were disabled
            else {
                
                pLedStates[1] = 1;
                LED_Set(1);
                AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
            }
        }
    }
}*/


int main(void)
{   
  TRACE_INFO("Firmware version 0.0.1 unstable\n\r");
  TRACE_DEBUG("Compiled: %s %s \n\r", __DATE__, __TIME__);
  TRACE_DEBUG("Main program start\n\r");
  static const Pin Buttons_pins[] = { PINS_PUSHBUTTONS };
  PIO_Configure(Buttons_pins, PIO_LISTSIZE(Buttons_pins));
  
  /*PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST);
  PIO_ConfigureIt(&pinPB1, (void (*)(const Pin *)) ISR_Bp1);
  PIO_ConfigureIt(&pinPB2, (void (*)(const Pin *)) ISR_Bp2);
  PIO_EnableIt(&pinPB1);
  PIO_EnableIt(&pinPB2);*/
  
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

  Commander cmd(&cmdVault);
  cmd.start();
      
  ADCDriver adc;
  delayMs(100);
  adc.start();

  /*PWMCDriver pwm;
  pwm.init();*/

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
      TRACE_DEBUG("----------------------------------------------------------------\n\r");
      TRACE_DEBUG("| SW1 | SW2 | JSU | JSD | JSL | JSR | JSS | TEMP | TRIM |  MIC |\n\r");
      TRACE_DEBUG("|  %c  |  %c  |  %c  |  %c  |  %c  |  %c  |  %c  | %4d |  %3d | %4d |\n\r",
                  sw1 ? '+' : '-',
                  sw2 ? '+' : '-',
                  joyup ? '+' : '-',
                  joydown ? '+' : '-',
                  joyleft ? '+' : '-',
                  joyright ? '+' : '-',
                  joysw ? '+' : '-',
                  adc.getTemp(),
                  adc.getTrim() * 100 / ADC_VREF,
                  adc.getMicIn());
      // Start here
      
      unsigned int temp = 46875 / (adc.getTrim() * 10000 / ADC_VREF);
      tcrc = temp;
      
      if(go_right)
      {
        cmdVault.requests.testreq &=~(1 << 2);
      }
      else if(go_left)
      {
        cmdVault.requests.testreq |= (1 << 2);
      };
      
      if(sw1)
      {
        cmdVault.requests.testreq |= (1 << 0);
        cmdVault.requests.testreq |= (1 << 1);        
      }
      else if(sw2)
      {
        cmdVault.requests.testreq &=~(1 << 0);
        cmdVault.requests.testreq &=~(1 << 1);
      }
      else if(joysw)
      {
        TRACE_DEBUG("I2C data read %X\n\r", cmdVault.status.teststat);
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
