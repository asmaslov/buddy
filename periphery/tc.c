#include "tc.h"

void TC_Configure(AT91S_TC *pTc, unsigned int mode)
{
  pTc->TC_CCR = AT91C_TC_CLKDIS;
  pTc->TC_IDR = 0xFFFFFFFF;
  pTc->TC_SR;
  pTc->TC_CMR = mode;
}

void TC_Start(AT91S_TC *pTc)
{
  pTc->TC_CCR = AT91C_TC_CLKEN | AT91C_TC_SWTRG;
}

void TC_Stop(AT91S_TC *pTc)
{
  pTc->TC_CCR = AT91C_TC_CLKDIS;
}

//------------------------------------------------------------------------------
/// Finds the best MCK divisor given the timer frequency and MCK. The result
/// is guaranteed to satisfy the following equation:
/// \pre
///   (MCK / (DIV * 65536)) <= freq <= (MCK / DIV)
/// \endpre
/// with DIV being the highest possible value.
/// \param freq  Desired timer frequency.
/// \param mck  Master clock frequency.
/// \param div  Divisor value.
/// \param tcclks  TCCLKS field value for divisor.
/// \return 1 if a proper divisor has been found; otherwise 0.
//------------------------------------------------------------------------------
unsigned char TC_FindMckDivisor(unsigned int freq,
                                unsigned int mck,
                                unsigned int *div,
                                unsigned int *tcclks)
{
  const unsigned int divisors[5] = {2, 8, 32, 128, 1024};
  unsigned int index = 0;
  while (freq < ((mck / divisors[index]) / 65536))
  {
    index++;
    if (index == 5)
    {
      return 0;
    }
  }
  while (index < 4)
  {
    if (freq > (mck / divisors[index + 1]))
    {
      break;
    }
    index++;
  }
  if (div)
  {
    *div = divisors[index];
  }
  if (tcclks)
  {
    *tcclks = index;
  }
  return 1;
}
