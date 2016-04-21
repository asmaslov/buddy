#ifndef ADCD_H
#define ADCD_H

#include "pio.h"

// ADC_AD0 pin definition.
#define PIN_ADC0_ADC0 {BIT27, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
// ADC_AD1 pin definition.
#define PIN_ADC0_ADC1 {BIT28, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
// ADC_AD2 pin definition.
#define PIN_ADC0_ADC2 {BIT29, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
// ADC_AD3 pin definition.
#define PIN_ADC0_ADC3 {BIT30, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
// Pins ADC
#define PINS_ADC PIN_ADC0_ADC0, PIN_ADC0_ADC1, PIN_ADC0_ADC2, PIN_ADC0_ADC3

static const Pin ADC_pins[] = {
  PINS_ADC,
};

class ADCDriver
{
  public:
    ADCDriver();
    ~ADCDriver();
};

#endif //#ifndef ADCD_H
