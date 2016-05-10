#ifndef PWMCD_H
#define PWMCD_H

#include "pio.h"

#define PWM_FREQUENCY 1000
#define MAX_DUTY_CYCLE 50
#define MIN_DUTY_CYCLE 0

#define PIN_PWMC_PWM0 {BIT19, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_PWMC_PWM1 {BIT20, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_PWMC_PWM2 {BIT21, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_PWMC_PWM3 {BIT22, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_PWMC PIN_PWMC_PWM0, PIN_PWMC_PWM1, PIN_PWMC_PWM2, PIN_PWMC_PWM3

#define PWM_SPEAKER 0
#define PWM_LCD_BRI 1
#define PWM_CONTROL 2

static const Pin PWMC_pins[] = { PIN_PWMC_PWM0, PIN_PWMC_PWM2 };

typedef struct _PWM {
  unsigned int count;
  unsigned int duty;
  unsigned char fadeIn;  
} PWM;
    
void pwm_enable(PWM *p);

void pwm_disable(void);

void pwm_beep(unsigned int freq, unsigned int ms);

#endif //#ifndef PWMCD_H
