#include "pwmcd.h"
#include "pwmc.h"
#include "pmc.h"
#include "aic.h"

#include "assert.h"

static PWM *pwm;

static void pwm_handler(void)
{
  SANITY_CHECK(pwm);
  if ((AT91C_BASE_PWMC->PWMC_ISR & AT91C_PWMC_CHID0) == AT91C_PWMC_CHID0)
  {
    pwm->count++;
    if (pwm->count == (PWM_FREQUENCY / (MAX_DUTY_CYCLE - MIN_DUTY_CYCLE)))
    {
      if (pwm->fadeIn)
      {
        pwm->duty++;
        if (pwm->duty == MAX_DUTY_CYCLE)
        {
          pwm->fadeIn = 0;
        }
      }
      else
      { 
        pwm->duty--;
        if (pwm->duty == MIN_DUTY_CYCLE)
        {
          pwm->fadeIn = 1;
        }
      }
      pwm->count = 0;
      PWMC_SetDutyCycle(PWM_SPEAKER, pwm->duty);
    }
  }
}

void pwm_enable(PWM *p)
{
  SANITY_CHECK(p);
  pwm = p;
  pwm->count = 0;
  pwm->duty = MIN_DUTY_CYCLE;
  pwm->fadeIn = 1;
  PIO_Configure(PWMC_pins, PIO_LISTSIZE(PWMC_pins));
  PMC_EnablePeripheral(AT91C_ID_PWMC);
  // Settings:
  // - 100kHz PWM period (PWM_FREQUENCY)
  // - 1s rise/fall time for the LED intensity
  // Set clock A to run at 100kHz * MAX_DUTY_CYCLE (clock B is not used)
  PWMC_ConfigureClocks(PWM_FREQUENCY * MAX_DUTY_CYCLE, 0, BOARD_MCK);
  // Configure PWMC channel for LED0 (left-aligned)
  PWMC_ConfigureChannel(PWM_SPEAKER, AT91C_PWMC_CPRE_MCKA, 0, 0);
  PWMC_SetPeriod(PWM_SPEAKER, MAX_DUTY_CYCLE);
  PWMC_SetDutyCycle(PWM_SPEAKER, MIN_DUTY_CYCLE);
  // Configure PWMC channel for LED1 (center-aligned, inverted polarity)
  /*PWMC_ConfigureChannel(PWM_LCD_BRI, AT91C_PWMC_CPRE_MCKA, AT91C_PWMC_CALG, AT91C_PWMC_CPOL);
  PWMC_SetPeriod(PWM_LCD_BRI, MAX_DUTY_CYCLE);
  PWMC_SetDutyCycle(PWM_LCD_BRI, MIN_DUTY_CYCLE);*/
  // Configure interrupt on channel #1
  AIC_ConfigureIT(AT91C_ID_PWMC, AT91C_AIC_PRIOR_LOWEST, pwm_handler);
  AIC_EnableIT(AT91C_ID_PWMC);
  PWMC_EnableChannelIt(PWM_SPEAKER);
  // Enable channel #1 and #2
  PWMC_EnableChannel(PWM_SPEAKER);
  //PWMC_EnableChannel(PWM_LCD_BRI);
}

void pwm_disable(void)
{
  PMC_DisablePeripheral(AT91C_ID_PWMC);
}
