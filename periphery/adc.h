/// !Usage
///
/// -# Configurate the pins for ADC
/// -# Initialize the ADC with ADC_Initialize().
/// -# Select the active channel using ADC_EnableChannel()
/// -# Start the conversion with ADC_StartConversion()
//  -# Wait the end of the conversion by polling status with ADC_GetStatus()
//  -# Finally, get the converted data using ADC_GetConvertedData()
///
//------------------------------------------------------------------------------
#ifndef ADC_H
#define ADC_H

#include "board.h"

#define ADC_CHANNEL_0 0
#define ADC_CHANNEL_1 1
#define ADC_CHANNEL_2 2
#define ADC_CHANNEL_3 3
#define ADC_CHANNEL_4 4
#define ADC_CHANNEL_5 5
#define ADC_CHANNEL_6 6
#define ADC_CHANNEL_7 7

// ADC clock frequency, at 10-bit resolution (in Hz)
#define ADC_MAX_CK_10BIT 5000000
// ADC clock frequency, at 8-bit resolution (in Hz)
#define ADC_MAX_CK_8BIT 8000000
// Startup time max, return from Idle mode (in µs)
#define ADC_STARTUP_TIME_MAX 20
// Track and hold Acquisition Time min (in ns)
#define ADC_TRACK_HOLD_TIME_MIN 600

#ifdef __cplusplus
extern "C" {
#endif

  extern void ADC_Initialize (AT91S_ADC *pAdc,
                              unsigned char idAdc,
                              unsigned char trgEn,
                              unsigned char trgSel,
                              unsigned char sleepMode,
                              unsigned char resolution,        
                              unsigned int mckClock,
                              unsigned int adcClock,
                              unsigned int startupTime,
                              unsigned int sampleAndHoldTime);
  
  extern unsigned int ADC_GetModeReg(AT91S_ADC *pAdc);
  
  extern void ADC_EnableChannel(AT91S_ADC *pAdc,
                                unsigned int channel);
  
  extern void ADC_DisableChannel (AT91S_ADC *pAdc,
                                  unsigned int channel);
  
  extern unsigned int ADC_GetChannelStatus(AT91S_ADC *pAdc);
  
  extern void ADC_StartConversion(AT91S_ADC *pAdc);
  
  extern void ADC_SoftReset(AT91S_ADC *pAdc);
  
  extern unsigned int ADC_GetLastConvertedData(AT91S_ADC *pAdc);
  
  extern unsigned int ADC_GetConvertedData(AT91S_ADC *pAdc,
                                           unsigned int channel);
  
  extern void ADC_EnableIt(AT91S_ADC *pAdc,
                           unsigned int flag);
  
  extern void ADC_EnableDataReadyIt(AT91S_ADC *pAdc);
  
  extern void ADC_DisableIt(AT91S_ADC *pAdc,
                            unsigned int flag);
  
  extern unsigned int ADC_GetStatus(AT91S_ADC *pAdc);
  
  extern unsigned int ADC_GetInterruptMaskStatus(AT91S_ADC *pAdc);
  
  extern unsigned int ADC_IsInterruptMasked(AT91S_ADC *pAdc,
                                            unsigned int flag);
  
  extern unsigned int ADC_IsStatusSet(AT91S_ADC *pAdc,
                                      unsigned int flag);
  
  extern unsigned char ADC_IsChannelInterruptStatusSet(unsigned int adc_sr,
                                                       unsigned int channel);

#ifdef __cplusplus
}
#endif

#endif //#ifndef ADC_H
