#include "twi.h"
#include "smplmath.h"
#include "assert.h"
#include "trace.h"

void TWI_ConfigureMaster(AT91S_TWI *pTwi,
                         unsigned int twck,
                         unsigned int mck)
{
  unsigned int ckdiv = 0;
  unsigned int cldiv;
  unsigned char ok = 0;
  
  TRACE_DEBUG("TWI_ConfigureMaster()\n\r");
  SANITY_CHECK(pTwi);
  #ifdef AT91C_TWI_SVEN
    pTwi->TWI_CR = AT91C_TWI_SVEN;
  #endif
  // Reset the TWI
  pTwi->TWI_CR = AT91C_TWI_SWRST;
  pTwi->TWI_RHR;
  // TWI Slave Mode Disabled, TWI Master Mode Disabled
  #ifdef AT91C_TWI_SVEN
    pTwi->TWI_CR = AT91C_TWI_SVDIS;
  #endif
  pTwi->TWI_CR = AT91C_TWI_MSDIS;
  // Set master mode
  pTwi->TWI_CR = AT91C_TWI_MSEN;
  // Configure clock
  while (!ok)
  {
    cldiv = ((mck / (2 * twck)) - 3) / power(2, ckdiv);
    if (cldiv <= 255)
    {
      ok = 1;
    }
    else
    {
      ckdiv++;
    }
  }
  ASSERT(ckdiv < 8, "-F- Cannot find valid TWI clock parameters\n\r");
  TRACE_DEBUG("Using CKDIV = %u and CLDIV/CHDIV = %u\n\r", ckdiv, cldiv);
  pTwi->TWI_CWGR = 0;
  pTwi->TWI_CWGR = (ckdiv << 16) | (cldiv << 8) | cldiv;
}

#ifdef AT91C_TWI_SVEN
  void TWI_ConfigureSlave(AT91S_TWI *pTwi,
                          unsigned char slaveAddress)
  {
    unsigned int i;
    // TWI software reset
    pTwi->TWI_CR = AT91C_TWI_SWRST;
    pTwi->TWI_RHR;
    // Wait at least 10 ms
    // TODO: Make honest delay_ms(10)
    for (i=0; i < 1000000; i++);
    // TWI Slave Mode Disabled, TWI Master Mode Disabled
    pTwi->TWI_CR = AT91C_TWI_SVDIS | AT91C_TWI_MSDIS;
    // Slave Address
    pTwi->TWI_SMR = 0;
    pTwi->TWI_SMR = (slaveAddress << 16) & AT91C_TWI_SADR;
    // SVEN: TWI Slave Mode Enabled
    pTwi->TWI_CR = AT91C_TWI_SVEN;
    // Wait at least 10 ms
    // TODO: Make honest delay_ms(10)
    for (i=0; i < 1000000; i++);
    ASSERT( (pTwi->TWI_CR & AT91C_TWI_SVDIS)!=AT91C_TWI_SVDIS, "Problem slave mode");
  }
#endif

void TWI_Stop(AT91S_TWI *pTwi)
{
  SANITY_CHECK(pTwi);
  pTwi->TWI_CR = AT91C_TWI_STOP;
}

void TWI_StartRead(AT91S_TWI *pTwi,
                   unsigned char address,
                   unsigned int iaddress,
                   unsigned char isize)
{
  TRACE_DEBUG("TWI_StartRead()\n\r");
  SANITY_CHECK(pTwi);
  SANITY_CHECK((address & 0x80) == 0);
  SANITY_CHECK((iaddress & 0xFF000000) == 0);
  SANITY_CHECK(isize < 4);
  // Set slave address and number of internal address bytes
  pTwi->TWI_MMR = 0;
  pTwi->TWI_MMR = (isize << 8) | AT91C_TWI_MREAD | (address << 16);
  // Set internal address bytes
  pTwi->TWI_IADR = 0;
  pTwi->TWI_IADR = iaddress;
  // Send START condition
  pTwi->TWI_CR = AT91C_TWI_START;
}

unsigned char TWI_ReadByte(AT91S_TWI *pTwi)
{
  SANITY_CHECK(pTwi);
  return pTwi->TWI_RHR;
}

void TWI_WriteByte(AT91S_TWI *pTwi, unsigned char byte)
{
  SANITY_CHECK(pTwi);
  pTwi->TWI_THR = byte;
}

void TWI_StartWrite(AT91S_TWI *pTwi,
                    unsigned char address,
                    unsigned int iaddress,
                    unsigned char isize,
                    unsigned char byte)
{
  TRACE_DEBUG("TWI_StartWrite()\n\r");
  SANITY_CHECK(pTwi);
  SANITY_CHECK((address & 0x80) == 0);
  SANITY_CHECK((iaddress & 0xFF000000) == 0);
  SANITY_CHECK(isize < 4);
  // Set slave address and number of internal address bytes
  pTwi->TWI_MMR = 0;
  pTwi->TWI_MMR = (isize << 8) | (address << 16);
  // Set internal address bytes
  pTwi->TWI_IADR = 0;
  pTwi->TWI_IADR = iaddress;
  // Write first byte to send
  TWI_WriteByte(pTwi, byte);
}

unsigned char TWI_ByteReceived(AT91S_TWI *pTwi)
{
  return ((pTwi->TWI_SR & AT91C_TWI_RXRDY) == AT91C_TWI_RXRDY);
}

unsigned char TWI_ByteSent(AT91S_TWI *pTwi)
{
  return ((pTwi->TWI_SR & AT91C_TWI_TXRDY) == AT91C_TWI_TXRDY);
}

unsigned char TWI_TransferComplete(AT91S_TWI *pTwi)
{
  return ((pTwi->TWI_SR & AT91C_TWI_TXCOMP) == AT91C_TWI_TXCOMP);
}

void TWI_EnableIt(AT91S_TWI *pTwi,
                  unsigned int sources)
{
  SANITY_CHECK(pTwi);
  SANITY_CHECK((sources & 0xFFFFF088) == 0);
  pTwi->TWI_IER = sources;
}

void TWI_DisableIt(AT91S_TWI *pTwi,
                   unsigned int sources)
{
  SANITY_CHECK(pTwi);
  SANITY_CHECK((sources & 0xFFFFF088) == 0);
  pTwi->TWI_IDR = sources;
}

unsigned int TWI_GetStatus(AT91S_TWI *pTwi)
{
  SANITY_CHECK(pTwi);
  return pTwi->TWI_SR;
}

unsigned int TWI_GetMaskedStatus(AT91S_TWI *pTwi)
{
  unsigned int status;
  SANITY_CHECK(pTwi);
  status = pTwi->TWI_SR;
  status &= pTwi->TWI_IMR;
  return status;
}

void TWI_SendSTOPCondition(AT91S_TWI *pTwi)
{
  SANITY_CHECK(pTwi);
  pTwi->TWI_CR |= AT91C_TWI_STOP;
}
