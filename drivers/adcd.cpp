#include "adcd.h"
#include "adc.h"
#include "aic.h"
#include "assert.h"

unsigned char ADCDriver::conversionDoneTemp;
unsigned char ADCDriver::conversionDoneTrim;
unsigned char ADCDriver::conversionDoneMicIn;
unsigned int ADCDriver::temp;
unsigned int ADCDriver::trim;
unsigned int ADCDriver::micIn;

ADCDriver::ADCDriver()
{
  conversionDoneTemp = 0;
  conversionDoneTrim = 0;
  conversionDoneMicIn = 0;
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
  //TRACE_DEBUG("status =0x%X\n\r", status);
  //TRACE_DEBUG("adc_imr=0x%X\n\r", ADC_GetInterruptMaskStatus(AT91C_BASE_ADC));
  if(ADC_IsChannelInterruptStatusSet(status, ADC_TEMP))
  {
    //TRACE_DEBUG("channel temp");
    ADC_DisableIt(AT91C_BASE_ADC, ADC_TEMP);
    temp = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_TEMP);
    conversionDoneTemp = 1;
  }  
  if(ADC_IsChannelInterruptStatusSet(status, ADC_TRIM))
  {
    //TRACE_DEBUG("channel trim");
    ADC_DisableIt(AT91C_BASE_ADC, ADC_TRIM);
    trim = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_TRIM);
    conversionDoneTrim = 1;
  }  
  if(ADC_IsChannelInterruptStatusSet(status, ADC_MIC_IN))
  {
    //TRACE_DEBUG("channel mic in");
    ADC_DisableIt(AT91C_BASE_ADC, ADC_MIC_IN);
    micIn = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_MIC_IN);
    conversionDoneMicIn = 1;
  }    
}

unsigned int ADCDriver::convertHex2mV(unsigned int valueToConvert)
{
  return((ADC_VREF * valueToConvert)/0x3FF);
}

void ADCDriver::convert(void)
{
  conversionDoneTemp = 0;
  conversionDoneTrim = 0;
  conversionDoneMicIn = 0;
  ADC_EnableIt(AT91C_BASE_ADC, ADC_TEMP);
  ADC_EnableIt(AT91C_BASE_ADC, ADC_TRIM);
  ADC_EnableIt(AT91C_BASE_ADC, ADC_MIC_IN);  
  ADC_StartConversion(AT91C_BASE_ADC);
}

unsigned int ADCDriver::getTemp(void)
{
  timeout = 0;
  while(!conversionDoneTemp && (++timeout < ADC_MAX_TIMEOUT));
  if (timeout == ADC_MAX_TIMEOUT)
  {
    TRACE_ERROR("ADC temp convertion timeout\n\r");
    return 0;
  }
  return convertHex2mV(temp);
}

unsigned int ADCDriver::getTrim(void)
{
  timeout = 0;
  while(!conversionDoneTrim && (++timeout < ADC_MAX_TIMEOUT));
  if (timeout == ADC_MAX_TIMEOUT)
  {
    TRACE_ERROR("ADC trim convertion timeout\n\r");
    return 0;
  }
  return convertHex2mV(trim);
}

unsigned int ADCDriver::getMicIn(void)
{
  timeout = 0;
  while(!conversionDoneMicIn && (++timeout < ADC_MAX_TIMEOUT));
  if (timeout == ADC_MAX_TIMEOUT)
  {
    TRACE_ERROR("ADC mic in convertion timeout\n\r");
    return 0;
  }
  return convertHex2mV(micIn);
}
