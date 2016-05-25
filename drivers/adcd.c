#include "adcd.h"
#include "adc.h"
#include "aic.h"
#include "assert.h"

static ADC *adc;

static void adc_handler(void)
{
  SANITY_CHECK(adc);
  unsigned int status;
  status = ADC_GetStatus(AT91C_BASE_ADC);
  if(ADC_IsChannelInterruptStatusSet(status, ADC_TEMP))
  {
    ADC_DisableIt(AT91C_BASE_ADC, ADC_TEMP);  
    adc->temp = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_TEMP);
  }  
  if(ADC_IsChannelInterruptStatusSet(status, ADC_TRIM))
  {
    ADC_DisableIt(AT91C_BASE_ADC, ADC_TRIM);
    adc->trim = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_TRIM);
  }  
  if(ADC_IsChannelInterruptStatusSet(status, ADC_MIC_IN))
  {
    ADC_DisableIt(AT91C_BASE_ADC, ADC_MIC_IN);
    adc->micIn = ADC_GetConvertedData(AT91C_BASE_ADC, ADC_MIC_IN);
  }
}

void adc_enable(ADC *a)
{
  SANITY_CHECK(a);
  adc = a;
  adc->temp = 0;
  adc->trim = 0;
  adc->micIn = 0;
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
  AIC_ConfigureIT(AT91C_ID_ADC, AT91C_AIC_PRIOR_LOWEST, adc_handler);
  AIC_EnableIT(AT91C_ID_ADC);
}

void adc_disable(void)
{
  AIC_DisableIT(AT91C_ID_ADC);
}

unsigned int adc_convertHex2mV(unsigned int valueToConvert)
{
  return((ADC_VREF * valueToConvert)/0x3FF);
}

void adc_work(void)
{
  ADC_EnableIt(AT91C_BASE_ADC, ADC_TEMP);
  ADC_EnableIt(AT91C_BASE_ADC, ADC_TRIM);
  ADC_EnableIt(AT91C_BASE_ADC, ADC_MIC_IN);  
  ADC_StartConversion(AT91C_BASE_ADC);
}

unsigned int adc_getTemp(void)
{
  SANITY_CHECK(adc);
  return adc_convertHex2mV(adc->temp);
}

unsigned int adc_getTrim(void)
{
  SANITY_CHECK(adc);  
  return adc_convertHex2mV(adc->trim);
}

unsigned int adc_getMicIn(void)
{
  SANITY_CHECK(adc);  
  return adc_convertHex2mV(adc->micIn);
}
