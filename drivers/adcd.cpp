#include "adcd.h"
#include "adc.h"

ADCDriver::ADCDriver()
{
  PIO_Configure(ADC_pins, PIO_LISTSIZE(ADC_pins)); 
  // TODO: Init ADC
  //void ADC_Initialize
}

ADCDriver::~ADCDriver()
{

}
