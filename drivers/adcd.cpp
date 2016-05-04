#include "adcd.h"
#include "adc.h"
#include "aic.h"
#include "assert.h"

unsigned int ADCDriver::temp;
unsigned int ADCDriver::trim;
unsigned int ADCDriver::micIn;

ADCDriver::ADCDriver()
{
  temp = 0;
  trim = 0;
  micIn = 0;
  //PIO_Configure(ADC_pins, PIO_LISTSIZE(ADC_pins)); 
  ADC_Initialize(AT91C_BASE_ADC,
                 AT91C_ID_ADC,
                 AT91C_ADC_TRGEN_DIS,
                 0,
                 AT91C_ADC_SLEEP_NORMAL_MODE,
                 AT91C_ADC_LOWRES_10_BIT,
                 BOARD_MCK,
                 ADC_MAX_CK_10BIT,
                 10,
                 1200);
  ADC_EnableChannel(AT91C_BASE_ADC, ADC_TEMP);
  ADC_EnableChannel(AT91C_BASE_ADC, ADC_TRIM);
  ADC_EnableChannel(AT91C_BASE_ADC, ADC_MIC_IN);
  AIC_ConfigureIT(AT91C_ID_ADC, 0, ADCDriver::driverISR);
  AIC_EnableIT(AT91C_ID_ADC);
}

ADCDriver::~ADCDriver()
{

}

void ADCDriver::driverISR(void)
{
  unsigned int status;
  status = ADC_GetStatus(AT91C_BASE_ADC);
  if(ADC_IsChannelInterruptStatusSet(status, ADC_TEMP))
  {
    temp = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_TEMP);
  }  
  if(ADC_IsChannelInterruptStatusSet(status, ADC_TRIM))
  {
    trim = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_TRIM);
  }  
  if(ADC_IsChannelInterruptStatusSet(status, ADC_MIC_IN))
  {
    micIn = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_MIC_IN);
  }    
  ADC_StartConversion(AT91C_BASE_ADC);
}

unsigned int ADCDriver::convertHex2mV(unsigned int valueToConvert)
{
  return((ADC_VREF * valueToConvert)/0x3FF);
}

void ADCDriver::start(void)
{
  ADC_EnableIt(AT91C_BASE_ADC, ADC_TEMP);
  ADC_EnableIt(AT91C_BASE_ADC, ADC_TRIM);
  ADC_EnableIt(AT91C_BASE_ADC, ADC_MIC_IN);  
  ADC_StartConversion(AT91C_BASE_ADC);
}

unsigned int ADCDriver::getTemp(void)
{
  return convertHex2mV(temp);
}

unsigned int ADCDriver::getTrim(void)
{
  return convertHex2mV(trim);
}

unsigned int ADCDriver::getMicIn(void)
{
  return convertHex2mV(micIn);
}
