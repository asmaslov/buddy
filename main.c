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

static const Pin NodPower_pin = {BIT6, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT};
static const Pin Clock_pin = {BIT22, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT};
static const Pin Buttons_pins[] = { PINS_PUSHBUTTONS };
static const Pin Joystick_pins[] = { PINS_JOYSTICK };

#define KOEFF_END 10
#define KOEFF_SLOW 20
#define KOEFF_FAST 50
#define STEP_MAX 5000
#define STEP_MIN 1000

volatile unsigned int ppin = 0;
volatile unsigned int tcrc = 100;
volatile unsigned int step = STEP_MIN;
volatile unsigned int koeff = KOEFF_END;

volatile unsigned char tickEnable = FALSE;
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
     
  if(tickEnable)
  {
    if(ppin == 1)
    {
      ppin = 0;
      PIO_Clear(&Clock_pin);
    }
    else
    {
      ppin = 1;
      PIO_Set(&Clock_pin);
    }
  }
  
  if(go_right)
  {
    step++;
    if(step > STEP_MAX - STEP_MIN)
    {
      koeff = KOEFF_END;
    }
    else if(step > STEP_MAX - (2 * STEP_MIN))
    {
      koeff = KOEFF_SLOW;
    }
    else      
    {
      koeff = KOEFF_FAST;
    }
    if(step > STEP_MAX)
    {
      go_right = FALSE;
      go_left = TRUE;
      commandVault_lock();
      commandVault.requests.endir12 &= ~(1 << 1);
      commandVault.requests.endir12 &= ~(1 << 3);
      commandVault.requests.endir34 &= ~(1 << 1);
      commandVault.requests.endir34 |= (1 << 3);
      commandVault_unlock();
    }
  }
  if(go_left)
  {
    step--;
    if(step < STEP_MIN + STEP_MIN)
    {
      koeff = KOEFF_END;
    }
    else if(step < STEP_MIN + (2 * STEP_MIN))
    {
      koeff = KOEFF_SLOW;
    }      
    else      
    {
      koeff = KOEFF_FAST;
    }
    if(step < STEP_MIN)
    {
      go_right = TRUE;
      go_left = FALSE;
      commandVault_lock();
      commandVault.requests.endir12 |= (1 << 1);
      commandVault.requests.endir12 |= (1 << 3);
      commandVault.requests.endir34 |= (1 << 1);
      commandVault.requests.endir34 &= ~(1 << 3);
      commandVault_unlock();
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
    AIC_ConfigureIT(AT91C_ID_TC0, AT91C_AIC_PRIOR_HIGHEST, ISR_Tc0);
    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;
    AIC_EnableIT(AT91C_ID_TC0);

    TC_Start(AT91C_BASE_TC0);
}

static void sw1Handler(void)
{
  TRACE_DEBUG("sw1 interrupt handler");
  if(!PIO_Get(&Buttons_pins[PUSHBUTTON_BP1]))
  {
    step = STEP_MIN;
    tickEnable = TRUE;
    commandVault_lock();
    commandVault.requests.endir12 |= (1 << 0);
    commandVault.requests.endir12 |= (1 << 2);
    commandVault.requests.endir34 |= (1 << 0);
    commandVault.requests.endir34 |= (1 << 2);
    commandVault.requests.endir12 |= (1 << 1);
    commandVault.requests.endir12 |= (1 << 3);
    commandVault.requests.endir34 |= (1 << 1);
    commandVault.requests.endir34 &= ~(1 << 3);
    commandVault_unlock();      
  }
}

static void sw2Handler(void)
{
  TRACE_DEBUG("sw2 interrupt handler");
  if(!PIO_Get(&Buttons_pins[PUSHBUTTON_BP2]))  
  {
    tickEnable = FALSE;
    commandVault.requests.endir12 &=~(1 << 0);
    commandVault.requests.endir12 &=~(1 << 2);
    commandVault.requests.endir34 &=~(1 << 0);
    commandVault.requests.endir34 &=~(1 << 2);
  }
}

int main(void)
{   
  TRACE_INFO("Firmware version 0.0.1 unstable\n\r");
  TRACE_DEBUG("Compiled: %s %s \n\r", __DATE__, __TIME__);
  TRACE_DEBUG("Program start\n\r");
  
  // Enable buttons and joystick
  PIO_Configure(Buttons_pins, PIO_LISTSIZE(Buttons_pins));
  unsigned char sw1, sw2;
  
  PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST);
  PIO_ConfigureIt(&Buttons_pins[PUSHBUTTON_BP1], (PinHandler)sw1Handler);
  PIO_ConfigureIt(&Buttons_pins[PUSHBUTTON_BP2], (PinHandler)sw2Handler);
  PIO_EnableIt(&Buttons_pins[PUSHBUTTON_BP1]);
  PIO_EnableIt(&Buttons_pins[PUSHBUTTON_BP2]);
 
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

  PIO_Configure(&NodPower_pin, 1);
  PIO_Configure(&Clock_pin, 1);

  TRACE_DEBUG("Initialization complete\n\r");
  
  // Power up I2C nods
  PIO_Set(&NodPower_pin);
  delayMs(200);
  
  // TODO: Make a nice standby mode instead of while(1)
  unsigned long fastTick = 0;
  unsigned long slowTick = 0;
  unsigned char looptrace = FALSE;
  
  // Main interrupt-based logic launch
  commander_start();
  
  delayMs(100);
  
  // Launch common step ticker
  ConfigureTc();
   
  while(1)
  {
    // Fast and furious
    fastTick++;
    if(fastTick > MAIN_LOOP_FAST_DELAY)
    {
      fastTick = 0;        
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
      
      // Start here
      tcrc = 46875 / (trimmer * koeff);
      if(commandVault.requests.buttonA)
      {
        TRACE_DEBUG("Button A\n\r");
      }
      if(commandVault.requests.buttonB)
      {
        TRACE_DEBUG("Button B\n\r");
      }
      // End here      
    }

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
