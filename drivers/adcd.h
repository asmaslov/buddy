#ifndef ADCD_H
#define ADCD_H

#include "pio.h"

#define PIN_ADC0_ADC0 {BIT27, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
#define PIN_ADC0_ADC1 {BIT28, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
#define PIN_ADC0_ADC2 {BIT29, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
#define PIN_ADC0_ADC3 {BIT30, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_INPUT, PIO_DEFAULT}
#define PINS_ADC PIN_ADC0_ADC0, PIN_ADC0_ADC1, PIN_ADC0_ADC2, PIN_ADC0_ADC3

#define ADC_VREF 3300 // 3.3 V

#define ADC_TEMP ADC_CHANNEL_5
#define ADC_TRIM ADC_CHANNEL_6
#define ADC_MIC_IN ADC_CHANNEL_7

#define ADC_MAX_TIMEOUT 100

static const Pin ADC_pins[] = {
  PINS_ADC,
};

class ADCDriver
{
  private:
    unsigned int timeout;
    static unsigned int temp;
    static unsigned int trim;
    static unsigned int micIn;
      
  private:
    static void driverISR(void);
    unsigned int convertHex2mV(unsigned int valueToConvert);
  
  public:
    ADCDriver();
    ~ADCDriver();
    void start(void);
    
  public:  
    unsigned int getTemp(void);
    unsigned int getTrim(void);
    unsigned int getMicIn(void);
};

#endif //#ifndef ADCD_H
