#include "usart.h"
#include "assert.h"

void USART_Configure(AT91S_USART *usart,
                     unsigned int mode,
                     unsigned int baudrate,
                     unsigned int masterClock)
{
  // Reset and disable receiver & transmitter
  usart->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RXDIS | AT91C_US_TXDIS;
  // Configure mode
  usart->US_MR = mode;
  // Configure baudrate
  // Asynchronous, no oversampling
  if (((mode & AT91C_US_SYNC) == 0) && ((mode & AT91C_US_OVER) == 0))
  {
    usart->US_BRGR = (masterClock / baudrate) / 16;
  }
  // TODO: other modes
}

void USART_SetTransmitterEnabled(AT91S_USART *usart,
                                 unsigned char enabled)
{
  if (enabled)
  {
    usart->US_CR = AT91C_US_TXEN;
  }
  else
  {
    usart->US_CR = AT91C_US_TXDIS;
  }
}

void USART_SetReceiverEnabled(AT91S_USART *usart,
                              unsigned char enabled)
{
  if (enabled)
  {
    usart->US_CR = AT91C_US_RXEN;
  }
  else
  {
    usart->US_CR = AT91C_US_RXDIS;
  }
}

void USART_Write(AT91S_USART *usart,
                 unsigned short data,
                 volatile unsigned int timeOut)
{
  if (timeOut == 0)
  {
    while ((usart->US_CSR & AT91C_US_TXEMPTY) == 0);
  }
  else
  {
    while ((usart->US_CSR & AT91C_US_TXEMPTY) == 0)
    {
      if (timeOut == 0)
      {
        TRACE_ERROR("USART_Write: Timed out.\n\r");
        return;
      }
      timeOut--;
    }
  }
  usart->US_THR = data;
}

unsigned char USART_WriteBuffer(AT91S_USART *usart,
                                void *buffer,
                                unsigned int size)
{
  // Check if the first PDC bank is free
  if ((usart->US_TCR == 0) && (usart->US_TNCR == 0))
  {
    usart->US_TPR = (unsigned int) buffer;
    usart->US_TCR = size;
    usart->US_PTCR = AT91C_PDC_TXTEN;
    return 1;
  }
  // Check if the second PDC bank is free
  else if (usart->US_TNCR == 0)
  {
    usart->US_TNPR = (unsigned int) buffer;
    usart->US_TNCR = size;
    return 1;
  }
  else
  {
    return 0;
  }
}

unsigned short USART_Read(AT91S_USART *usart,
                          volatile unsigned int timeOut)
{
  if (timeOut == 0)
  {
    while ((usart->US_CSR & AT91C_US_RXRDY) == 0);
  }
  else
  {
    while ((usart->US_CSR & AT91C_US_RXRDY) == 0)
    {
      if (timeOut == 0)
      {
        TRACE_ERROR("USART_Read: Timed out.\n\r");
        return 0;
      }
      timeOut--;
    }
  }
  return usart->US_RHR;
}

unsigned char USART_ReadBuffer(AT91S_USART *usart,
                                      void *buffer,
                                      unsigned int size)
{
  // Check if the first PDC bank is free
  if ((usart->US_RCR == 0) && (usart->US_RNCR == 0))
  {
    usart->US_RPR = (unsigned int) buffer;
    usart->US_RCR = size;
    usart->US_PTCR = AT91C_PDC_RXTEN;
    return 1;
  }
  // Check if the second PDC bank is free
  else if (usart->US_RNCR == 0)
  {
    usart->US_RNPR = (unsigned int) buffer;
    usart->US_RNCR = size;
    return 1;
  }
  else
  {
    return 0;
  }
}

unsigned char USART_IsDataAvailable(AT91S_USART *usart)
{
  if ((usart->US_CSR & AT91C_US_RXRDY) != 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void USART_SetIrdaFilter(AT91S_USART *pUsart,
                         unsigned char filter)
{
  SANITY_CHECK(pUsart);
  pUsart->US_IF = filter;
}
