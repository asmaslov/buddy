#include "pwmcd.h"
#include "pwmc.h"
#include "pmc.h"
#include "aic.h"

unsigned int PWMCDriver::count;
unsigned int PWMCDriver::duty;
unsigned char PWMCDriver::fadeIn;

PWMCDriver::PWMCDriver()
{
  count = 0;
  duty = MIN_DUTY_CYCLE;
  fadeIn = 1;
  PIO_Configure(PWMC_pins, PIO_LISTSIZE(PWMC_pins));
  PMC_EnablePeripheral(AT91C_ID_PWMC);
}

PWMCDriver::~PWMCDriver()
{
  PMC_DisablePeripheral(AT91C_ID_PWMC);
}

void PWMCDriver::driverISR(void)
{
  if ((AT91C_BASE_PWMC->PWMC_ISR & AT91C_PWMC_CHID0) == AT91C_PWMC_CHID0)
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
      PWMC_SetDutyCycle(PWM_SPEAKER, duty);
      //PWMC_SetDutyCycle(PWM_LCD_BRI, duty);
    }
  }
}

void PWMCDriver::init(void)
{
  // Settings:
  // - 100kHz PWM period (PWM_FREQUENCY)
  // - 1s rise/fall time for the LED intensity
  // Set clock A to run at 100kHz * MAX_DUTY_CYCLE (clock B is not used)
  PWMC_ConfigureClocks(PWM_FREQUENCY * MAX_DUTY_CYCLE, 0, BOARD_MCK);
  // Configure PWMC channel for LED0 (left-aligned)
  //PWMC_ConfigureChannel(PWM_SPEAKER, AT91C_PWMC_CPRE_MCKA, 0, 0);
  PWMC_ConfigureChannel(PWM_SPEAKER, AT91C_PWMC_CPRE_MCKA, AT91C_PWMC_CALG, AT91C_PWMC_CPOL);
  PWMC_SetPeriod(PWM_SPEAKER, MAX_DUTY_CYCLE);
  PWMC_SetDutyCycle(PWM_SPEAKER, MIN_DUTY_CYCLE);
  // Configure PWMC channel for LED1 (center-aligned, inverted polarity)
  /*PWMC_ConfigureChannel(PWM_LCD_BRI, AT91C_PWMC_CPRE_MCKA, AT91C_PWMC_CALG, AT91C_PWMC_CPOL);
  PWMC_SetPeriod(PWM_LCD_BRI, MAX_DUTY_CYCLE);
  PWMC_SetDutyCycle(PWM_LCD_BRI, MIN_DUTY_CYCLE);*/
  // Configure interrupt on channel #1
  AIC_ConfigureIT(AT91C_ID_PWMC, AT91C_AIC_PRIOR_LOWEST, PWMCDriver::driverISR);
  AIC_EnableIT(AT91C_ID_PWMC);
  PWMC_EnableChannelIt(PWM_SPEAKER);
  // Enable channel #1 and #2
  PWMC_EnableChannel(PWM_SPEAKER);
  //PWMC_EnableChannel(PWM_LCD_BRI);
}
