#ifndef PWMCD_H
#define PWMCD_H

#include "pio.h"

#define PWM_FREQUENCY 20000
#define MAX_DUTY_CYCLE 50
#define MIN_DUTY_CYCLE 0

#define PIN_PWMC_PWM0 {BIT19, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_PWMC_PWM1 {BIT20, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_PWMC_PWM2 {BIT21, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_PWMC_PWM3 {BIT22, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_PWMC PIN_PWMC_PWM0, PIN_PWMC_PWM1, PIN_PWMC_PWM2, PIN_PWMC_PWM3

#define CHANNEL_PWM0 0
#define CHANNEL_PWM1 1
#define CHANNEL_PWM2 2
#define CHANNEL_PWM3 3

static const Pin PWMC_pins[] = { PIN_PWMC_PWM0 };

class PWMCDriver
{
  private:
    static void driverISR(void);
  
  public:
    PWMCDriver();
    ~PWMCDriver();
};

void initPWMD(void);

#endif //#ifndef PWMCD_H
