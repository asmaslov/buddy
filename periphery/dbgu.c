#include "dbgu.h"

void DBGU_Configure(unsigned int mode,
                    unsigned int baudrate,
                    unsigned int mck)
{   
  // Reset & disable receiver and transmitter, disable interrupts
  AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RSTRX | AT91C_US_RSTTX;
  AT91C_BASE_DBGU->DBGU_IDR = 0xFFFFFFFF;   
  // Configure baud rate
  AT91C_BASE_DBGU->DBGU_BRGR = mck / (baudrate * 16);
  // Configure mode register
  AT91C_BASE_DBGU->DBGU_MR = mode;
  // Disable DMA channel
  AT91C_BASE_DBGU->DBGU_PTCR = AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS;
  // Enable receiver and transmitter
  AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RXEN | AT91C_US_TXEN;
}

void DBGU_PutChar(unsigned char c)
{
  // Wait for the transmitter to be ready
  while ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXEMPTY) == 0);    
  // Send character
  AT91C_BASE_DBGU->DBGU_THR = c;
  // Wait for the transfer to complete
  while ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXEMPTY) == 0);
}

unsigned int DBGU_IsRxReady()
{
  return (AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_RXRDY);
}

unsigned char DBGU_GetChar(void)
{
  while ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_RXRDY) == 0);
  return AT91C_BASE_DBGU->DBGU_RHR;
}

#ifndef NOFPUT
  #include "stdio.h"

  signed int fputc(signed int c, FILE *pStream)
  {
    if ((pStream == stdout) || (pStream == stderr))
    {
      DBGU_PutChar(c);
      return c;
    }
    else
    {
      return EOF;
    }
  }

  signed int fputs(const char *pStr, FILE *pStream)
  {
    signed int num = 0;
    while (*pStr != 0)
    {
      if (fputc(*pStr, pStream) == -1)
      {
        return -1;
      }
      num++;
      pStr++;
    }
    return num;
  }

  #undef putchar
  signed int putchar(signed int c)
  {
    return fputc(c, stdout);
  }

#endif //#ifndef NOFPUT
