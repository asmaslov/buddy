#include "pio.h"
#include "aic.h"
#include "pit.h"
#include "assert.h"
#include "adcd.h"
#include "pwmcd.h"
#include "lcd.h"
#include "usartd.h"
#include "comvault.h"
#include "commander.h"
#include "manipulator.h"

#include "delay.h"

#include <stdio.h>

#define PIT_PERIOD_US 1000 
#define DEBOUNCE_TIME 200 // Times of PIT_PERIOD_US

#define MAIN_LOOP_SLOW_DELAY 100000
#define MAIN_LOOP_FAST_DELAY 100

#define MAX_TRIM_VALUE 100

static const Pin Buttons_pins[] = { PINS_PUSHBUTTONS };
static const Pin Joystick_pins[] = { PINS_JOYSTICK };
static unsigned long sw1HandlerTimestamp = 0;
static unsigned long sw2HandlerTimestamp = 0;
static unsigned long timestamp = 0;

// Global objects
Manipulator manipulator;
CommandVault commandVault;
Comport comport;
Commander commander;
CommanderTicker commanderTicker;
// --

static void sw1Handler(void)
{
  if(!PIO_Get(&Buttons_pins[PUSHBUTTON_BP1]))
  {
    if((timestamp - sw1HandlerTimestamp) > DEBOUNCE_TIME)
    {
      sw1HandlerTimestamp = timestamp;
      TRACE_INFO("Manipulator motors enabled\n\r");
      manipulator_unfreeze();
    }    
  }
}

static void sw2Handler(void)
{
  if(!PIO_Get(&Buttons_pins[PUSHBUTTON_BP2]))  
  {
    if((timestamp - sw2HandlerTimestamp) > DEBOUNCE_TIME)
    {
      sw2HandlerTimestamp = timestamp;
      TRACE_INFO("Manipulator motors disabled\n\r");
      manipulator_freeze();
    }    
  }
}

static void systemTicker(void)
{
  unsigned int status;
  status = PIT_GetStatus() & AT91C_PITC_PITS;
  if (status != 0)
  {
    timestamp += (PIT_GetPIVR() >> 20);
  }
}

void configurePit(void)
{
  PIT_Init(PIT_PERIOD_US, BOARD_MCK / 1000000);
  AIC_DisableIT(AT91C_ID_SYS);
  AIC_ConfigureIT(AT91C_ID_SYS, AT91C_AIC_PRIOR_LOWEST, systemTicker);
  AIC_EnableIT(AT91C_ID_SYS);
  PIT_EnableIT();
  PIT_Enable();
}

int main(void)
{   
  TRACE_INFO("Firmware version 0.0.1 unstable\n\r");
  TRACE_DEBUG("Compiled: %s %s \n\r", __DATE__, __TIME__);
  TRACE_DEBUG("Program start\n\r");
  
  // Start system ticker
  configurePit();
  // --
  
  // Enable buttons and joystick, configure interrupts
  PIO_Configure(Buttons_pins, PIO_LISTSIZE(Buttons_pins));
  unsigned char sw1, sw2;
  PIO_Configure(Joystick_pins, PIO_LISTSIZE(Joystick_pins));
  unsigned char joyup, joydown, joyleft, joyright, joysw;
  PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST);
  PIO_ConfigureIt(&Buttons_pins[PUSHBUTTON_BP1], (PinHandler)sw1Handler);
  PIO_ConfigureIt(&Buttons_pins[PUSHBUTTON_BP2], (PinHandler)sw2Handler);
  // ---
  
  // Create and enable periphery modules
  ADC adc;
  adc_enable(&adc);
  unsigned int temperature, trimmer, microphone;
  adc_work();
  //PWM pwm;
  //pwm_enable(&pwm);
  comport_enable(&comport);
  comport_configure(USART0, BAUDRATE);
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

  // Launch main logic modules, order is important
  commandVault_init(&commandVault);
  commanderTicker = commander_init(&commander, &commandVault, &comport);
  manipulator_init(&manipulator, &commander, &commandVault, &comport);
  manipulator_configure(commanderTicker);
  commander_nodsPowerUp();
  commander_start();
  manipulator_startParser();
  
  TRACE_DEBUG("Initialization complete\n\r");
  // --
  
  // Enable controls
  PIO_EnableIt(&Buttons_pins[PUSHBUTTON_BP1]);
  PIO_EnableIt(&Buttons_pins[PUSHBUTTON_BP2]);
  // --
  
  // TODO:
  // Use FreeRTOS
  unsigned long fastTick = 0;
  unsigned long slowTick = 0;
  unsigned char looptrace = FALSE; 
  
  while(TRUE)
  {
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
      trimmer = adc_getTrim() * MAX_TRIM_VALUE / ADC_VREF;
      microphone = adc_getMicIn();
      adc_work();      

      // Fast operations
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
      
      manipulator.globalSpeedMultiplier = trimmer;


      
      if(commandVault.holdkeys.buttonA)
      {
        TRACE_DEBUG("Button A\n\r");
      }
      if(commandVault.holdkeys.buttonB)
      {
        TRACE_DEBUG("Button B\n\r");
      }
      // --
    }
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

      // Slow operations
    
      // --
    }
  }
}
