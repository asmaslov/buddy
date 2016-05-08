#ifndef ADCD_H
#define ADCD_H

#include "pio.h"

#define PIN_ADC0_ADC0 {BIT27, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
#define PIN_ADC0_ADC1 {BIT28, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
#define PIN_ADC0_ADC2 {BIT29, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
#define PIN_ADC0_ADC3 {BIT30, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
#define PINS_ADC PIN_ADC0_ADC0, PIN_ADC0_ADC1, PIN_ADC0_ADC2, PIN_ADC0_ADC3

#define ADC_VREF 3300 // mV

#define ADC_TEMP ADC_CHANNEL_5
#define ADC_TRIM ADC_CHANNEL_6
#define ADC_MIC_IN ADC_CHANNEL_7

#define ADC_MAX_TIMEOUT 100

static const Pin ADC_pins[] = {
  PINS_ADC,
};

typedef struct _ADC {
  unsigned int timeout;
  unsigned int temp;
  unsigned int trim;
  unsigned int micIn;
} ADC;

void adc_enable(ADC *a);

void adc_disable(void);

unsigned int adc_convertHex2mV(unsigned int valueToConvert);

void adc_work(void);

unsigned int adc_getTemp(void);

unsigned int adc_getTrim(void);

unsigned int adc_getMicIn(void);

#endif //#ifndef ADCD_H
