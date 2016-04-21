#include "pwmcd.h"
#include "pwmc.h"

// PWM Setup
#define PWM_FREQUENCY 20000
#define MAX_DUTY_CYCLE 50
#define MIN_DUTY_CYCLE 0

static void ISR_Pwmc(void)
{
  static unsigned int count = 0;
  static unsigned int duty = MIN_DUTY_CYCLE;
  static unsigned char fadeIn = 1;
  // Interrupt on channel #1
  if ((AT91C_BASE_PWMC->PWMC_ISR & AT91C_PWMC_CHID1) == AT91C_PWMC_CHID1)
  {
    count++;
    if (count == (PWM_FREQUENCY / (MAX_DUTY_CYCLE - MIN_DUTY_CYCLE)))
    {
      if (fadeIn)
      {
        duty++;
        if (duty == MAX_DUTY_CYCLE)
        {
          fadeIn = 0;
        }
      }
      else
      { 
        duty--;
        if (duty == MIN_DUTY_CYCLE)
        {
          fadeIn = 1;
        }
      }
      count = 0;
      //PWMC_SetDutyCycle(CHANNEL_PWM_LED0, duty);
      //PWMC_SetDutyCycle(CHANNEL_PWM_LED1, duty);
    }
  }
}

/*void initPWMD(void)
{
  PIO_Configure(PWMC_pins, PIO_LISTSIZE(PWMC_pins));
  // Enable PWMC peripheral clock
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PWMC;
  // Settings:
  // - 100kHz PWM period (PWM_FREQUENCY)
  // - 1s rise/fall time for the LED intensity
  // Set clock A to run at 100kHz * MAX_DUTY_CYCLE (clock B is not used)
  PWMC_ConfigureClocks(PWM_FREQUENCY * MAX_DUTY_CYCLE, 0, BOARD_MCK);
  // Configure PWMC channel for LED0 (left-aligned)
  PWMC_ConfigureChannel(CHANNEL_PWM_LED0, AT91C_PWMC_CPRE_MCKA, 0, 0);
  PWMC_SetPeriod(CHANNEL_PWM_LED0, MAX_DUTY_CYCLE);
  PWMC_SetDutyCycle(CHANNEL_PWM_LED0, MIN_DUTY_CYCLE);
  // Configure PWMC channel for LED1 (center-aligned, inverted polarity)
  PWMC_ConfigureChannel(CHANNEL_PWM_LED1, AT91C_PWMC_CPRE_MCKA, AT91C_PWMC_CALG, AT91C_PWMC_CPOL);
  PWMC_SetPeriod(CHANNEL_PWM_LED1, MAX_DUTY_CYCLE);
  PWMC_SetDutyCycle(CHANNEL_PWM_LED1, MIN_DUTY_CYCLE);
  // Configure interrupt on channel #1
  AIC_ConfigureIT(AT91C_ID_PWMC, 0, ISR_Pwmc);
  AIC_EnableIT(AT91C_ID_PWMC);
  PWMC_EnableChannelIt(CHANNEL_PWM_LED0);
  // Enable channel #1 and #2
  PWMC_EnableChannel(CHANNEL_PWM_LED0);
  PWMC_EnableChannel(CHANNEL_PWM_LED1);
}*/