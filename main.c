#include "assert.h"
#include "adcd.h"
#include "pwmcd.h"

#include "pio.h"

#include "tc.h"
#include "aic.h"

#include "lcd.h"
#include "usartd.h"

#include "comvault.h"
#include "commander.h"

#include "delay.h"

#include <stdio.h>

#define MAIN_LOOP_SLOW_DELAY 100000
#define MAIN_LOOP_FAST_DELAY 100

static const Pin Test_pin = {BIT6, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT};
static const Pin Clock_pin = {BIT22, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT};

#define STEP_MAX 700
#define STEP_MIN 50

volatile unsigned int ppin = 0;
volatile unsigned int tcrc = 100;
volatile unsigned int step = STEP_MIN;

volatile unsigned char go_right = TRUE;
volatile unsigned char go_left = FALSE;
  
CommandVault commandVault;
Comport comport;

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
      PIO_Clear(&Clock_pin);
      PIO_Clear(&Test_pin);
    }
    else
    {
      ppin = 1;
      PIO_Set(&Clock_pin);
      PIO_Set(&Test_pin);
    }

    if(go_right)
    {
      step++;
      if(step > STEP_MAX)
      {
        go_right = FALSE;
        go_left = TRUE;
      }
    }
    if(go_left)
    {
      step--;
      if(step < STEP_MIN)
      {
        go_right = TRUE;
        go_left = FALSE;
      }
    }  
    
    
    commandVault_lock();
    if(go_right)
    {
      commandVault.requests.endir34 |= (1 << 1);
    }
    if(go_left)
    {
      commandVault.requests.endir34 &= ~(1 << 1);
    }
    comport_uputchar(commandVault.status.stat12 >> 6);
    comport_uputchar(commandVault.status.stat34 >> 6);
    commandVault_unlock();
    
    
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
    AIC_ConfigureIT(AT91C_ID_TC0, AT91C_AIC_PRIOR_HIGHEST, ISR_Tc0);
    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;
    AIC_EnableIT(AT91C_ID_TC0);

    TC_Start(AT91C_BASE_TC0);
}

static void sw1Handler(void)
{
  TRACE_DEBUG("sw1 interrupt handler");
  
}

static void sw2Handler(void)
{
  TRACE_DEBUG("sw2 interrupt handler");

}

int main(void)
{   
  TRACE_INFO("Firmware version 0.0.1 unstable\n\r");
  TRACE_DEBUG("Compiled: %s %s \n\r", __DATE__, __TIME__);
  TRACE_DEBUG("Program start\n\r");
  
  // Enable buttons and joystick
  static const Pin Buttons_pins[] = { PINS_PUSHBUTTONS };
  PIO_Configure(Buttons_pins, PIO_LISTSIZE(Buttons_pins));
  unsigned char sw1, sw2;
  
  PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST);
  PIO_ConfigureIt(&Buttons_pins[PUSHBUTTON_BP1], (PinHandler)sw1Handler);
  PIO_ConfigureIt(&Buttons_pins[PUSHBUTTON_BP2], (PinHandler)sw2Handler);
  PIO_EnableIt(&Buttons_pins[PUSHBUTTON_BP1]);
  PIO_EnableIt(&Buttons_pins[PUSHBUTTON_BP2]);
  
  static const Pin Joystick_pins[] = { PINS_JOYSTICK };
  PIO_Configure(Joystick_pins, PIO_LISTSIZE(Joystick_pins));
  unsigned char joyup, joydown, joyleft, joyright, joysw;
  // ---
  
  // Create and enable periphery modules
  ADC adc;
  adc_enable(&adc);
  adc_work();
  unsigned int temperature, trimmer, microphone;  
  

  
  PWM pwm;
  pwm_enable(&pwm);   
  // ---
  
  // User hello
  delayMs(100);
  LCD_init();
  delayUs(100);
  LCDSettings(); 
  delayUs(100);
  LCDWrite130x130bmp();
  delayMs(500);
  // ---

  // Create and enable main logic modules

  commandVault_init(&commandVault);
  
  comport_enable(&comport);
  comport_configure(USART0, 57600);
  
  Commander commander;
  commander_init(&commander, &commandVault, &comport);
  // ---

  PIO_Configure(&Test_pin, 1);
  PIO_Configure(&Clock_pin, 1);
  ConfigureTc();
     
  TRACE_DEBUG("Initialization complete\n\r");
  
  // TODO: Make a nice standby mode instead of while(1)
  unsigned long fastTick = 0;
  unsigned long slowTick = 0;
  unsigned char looptrace = FALSE;
  
  // Main interrupt-based logic launch
  commander_start();
  
  commandVault.requests.endir12 = 0x3F;
  commandVault.requests.endir34 = 0x3F;
  
  while(1)
  {
    // Fast and furious
    fastTick++;
    if(fastTick > MAIN_LOOP_FAST_DELAY)
    {
      fastTick = 0;        
      /*commandVault_lock();
      if(go_right)
      {
        commandVault.requests.testreq &=~(1 << 2);
      }
      if(go_left)
      {
        commandVault.requests.testreq |= (1 << 2);
      }
      commandVault_unlock();*/
      sw1 = !PIO_Get(&Buttons_pins[PUSHBUTTON_BP1]);
      sw2 = !PIO_Get(&Buttons_pins[PUSHBUTTON_BP2]);
      joyup = !PIO_Get(&Joystick_pins[JOYSTICK_UP]);
      joydown = !PIO_Get(&Joystick_pins[JOYSTICK_DOWN]);
      joyleft = !PIO_Get(&Joystick_pins[JOYSTICK_LEFT]);
      joyright = !PIO_Get(&Joystick_pins[JOYSTICK_RIGHT]);
      joysw = !PIO_Get(&Joystick_pins[JOYSTICK_BUTTON]);
      temperature = adc_getTemp();
      trimmer = adc_getTrim() * 100 / ADC_VREF;
      microphone = adc_getMicIn();
      adc_work();
      if(sw1 && sw2 && joyup)
      {
        if(!looptrace)
        {
          TRACE_DEBUG("Monitoring enabled\n\r");
        }
        looptrace = TRUE;
      }
      if(sw1 && sw2 && joydown)
      {
        if(looptrace)
        {
          TRACE_DEBUG("Monitoring disabled\n\r");
        }
        looptrace = FALSE;
      }
    }
    
    // Start here
    tcrc = 46875 / (trimmer * 20);
    if(sw1)
    {
      commandVault.requests.endir34 |= (1 << 0);
    }
    if(sw2)
    {
      commandVault.requests.endir34 &=~(1 << 0);
    }
    if(commandVault.requests.buttonA)
    {
      TRACE_DEBUG("Button A\n\r");
    }
    if(commandVault.requests.buttonB)
    {
      TRACE_DEBUG("Button B\n\r");
    }
    // End here
    
    // Slow and happy
    slowTick++;
    if((slowTick > MAIN_LOOP_SLOW_DELAY) && looptrace)
    {
      slowTick = 0;
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
                  temperature, trimmer, microphone);

      // Start here
    
      // End here
    }
  }
}
