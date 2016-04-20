#ifndef PWMC_H
#define PWMC_H

#ifdef __cplusplus
extern "C" {
#endif

  extern void PWMC_ConfigureChannel(unsigned char channel,
                                    unsigned int prescaler,
                                    unsigned int alignment,
                                    unsigned int polarity);

  extern void PWMC_ConfigureClocks(unsigned int clka,
                                   unsigned int clkb,
                                   unsigned int mck);

  extern void PWMC_SetPeriod(unsigned char channel,
                             unsigned short period);

  extern void PWMC_SetDutyCycle(unsigned char channel,
                                unsigned short duty);

  extern void PWMC_EnableChannel(unsigned char channel);

  extern void PWMC_DisableChannel(unsigned char channel);

  extern void PWMC_EnableChannelIt(unsigned char channel);

  extern void PWMC_DisableChannelIt(unsigned char channel);

#ifdef __cplusplus
}
#endif

#endif //#ifndef PWMC_H
