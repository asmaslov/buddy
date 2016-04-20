#include "pwmc.h"
#include "board.h"
#include "assert.h"

static unsigned short FindClockConfiguration(unsigned int frequency,
                                             unsigned int mck)
{
  unsigned int divisors[11] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
  unsigned char divisor = 0;
  unsigned int prescaler;
  SANITY_CHECK(frequency < mck);
  prescaler = (mck / divisors[divisor]) / frequency;
  while ((prescaler > 255) && (divisor < 11))
  {
    divisor++;
    prescaler = (mck / divisors[divisor]) / frequency;
  }
  if (divisor < 11)
  {
    TRACE_DEBUG("Found divisor=%u and prescaler=%u for freq=%uHz\n\r",
                divisors[divisor], prescaler, frequency);
    return prescaler | (divisor << 8);
  }
  else
  {
    return 0;
  }
}

void PWMC_ConfigureChannel(unsigned char channel,
                           unsigned int prescaler,
                           unsigned int alignment,
                           unsigned int polarity)
{
  SANITY_CHECK(prescaler < AT91C_PWMC_CPRE_MCKB);
  SANITY_CHECK((alignment & ~AT91C_PWMC_CALG) == 0);
  SANITY_CHECK((polarity & ~AT91C_PWMC_CPOL) == 0);
  // Disable channel (effective at the end of the current period)
  if ((AT91C_BASE_PWMC->PWMC_SR & (1 << channel)) != 0)
  {
    AT91C_BASE_PWMC->PWMC_DIS = 1 << channel;
    while ((AT91C_BASE_PWMC->PWMC_SR & (1 << channel)) != 0);
  }
  // Configure channel
  AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CMR = prescaler | alignment | polarity;
}

void PWMC_ConfigureClocks(unsigned int clka,
                          unsigned int clkb,
                          unsigned int mck)
{
  unsigned int mode = 0;
  unsigned int result;
  // Clock A
  if (clka != 0)
  {
    result = FindClockConfiguration(clka, mck);
    ASSERT(result != 0, "-F- Could not generate the desired PWM frequency (%uHz)\n\r", clka);
    mode |= result;
  }
  // Clock B
  if (clkb != 0)
  {
    result = FindClockConfiguration(clkb, mck);
    ASSERT(result != 0, "-F- Could not generate the desired PWM frequency (%uHz)\n\r", clkb);
    mode |= (result << 16);
  }
  // Configure clocks
  TRACE_DEBUG("Setting PWMC_MR = 0x%08X\n\r", mode);
  AT91C_BASE_PWMC->PWMC_MR = mode;
}

void PWMC_SetPeriod(unsigned char channel,
                    unsigned short period)
{
  // If channel is disabled, write to CPRD
  if ((AT91C_BASE_PWMC->PWMC_SR & (1 << channel)) == 0)
  {
    AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CPRDR = period;
  }
  // Otherwise use update register
  else
  {
    AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CMR |= AT91C_PWMC_CPD;
    AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CUPDR = period;
  }
}

void PWMC_SetDutyCycle(unsigned char channel,
                       unsigned short duty)
{
  SANITY_CHECK(duty <= AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CPRDR);
  // If channel is disabled, write to CDTY
  if ((AT91C_BASE_PWMC->PWMC_SR & (1 << channel)) == 0)
  {
    AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CDTYR = duty;
  }
  // Otherwise use update register
  else
  {
    AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CMR &= ~AT91C_PWMC_CPD;
    AT91C_BASE_PWMC->PWMC_CH[channel].PWMC_CUPDR = duty;
  }
}

void PWMC_EnableChannel(unsigned char channel)
{
  AT91C_BASE_PWMC->PWMC_ENA = 1 << channel;
}

void PWMC_DisableChannel(unsigned char channel)
{
  AT91C_BASE_PWMC->PWMC_DIS = 1 << channel;
}

void PWMC_EnableChannelIt(unsigned char channel)
{
  AT91C_BASE_PWMC->PWMC_IER = 1 << channel;
}

void PWMC_DisableChannelIt(unsigned char channel)
{
  AT91C_BASE_PWMC->PWMC_IDR = 1 << channel;
}
