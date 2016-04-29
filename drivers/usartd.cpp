#include "usartd.h"
#include "usart.h"
#include "pmc.h"
#include "aic.h"
#include "assert.h"

#include <iostream>
#include <stdarg.h>

unsigned char USARTDriver::readBuffer[BUFFER_SIZE];
ParserFunc USARTDriver::parser;

USARTDriver::USARTDriver()
{
  PIO_Configure(USART0_pins, PIO_LISTSIZE(USART0_pins)); 
  PMC_EnablePeripheral(AT91C_ID_US0);
}

USARTDriver::~USARTDriver()
{
  PMC_DisablePeripheral(AT91C_ID_US0);
}

void USARTDriver::defaultISR0(void)
{
  AIC_ClearIT(AT91C_ID_US0);
  unsigned int status;
  status = AT91C_BASE_US0->US_CSR;
  if ((status & AT91C_US_RXBUFF) == AT91C_US_RXBUFF)
  {
    // TODO: Here we must copy bytes from buffer to parser
    if(parser)
    {
      parser(readBuffer, BUFFER_SIZE);
    }
    USART_ReadBuffer(AT91C_BASE_US0, readBuffer, BUFFER_SIZE);
  }
  AIC_FinishIT();
}

void USARTDriver::configure(unsigned char portnum, unsigned int speed)
{
  unsigned int mode, ipt;
  switch(portnum)
  {
    case USART0:
      this->port = USART0;
      mode = AT91C_US_USMODE_NORMAL
           | AT91C_US_CLKS_CLOCK
           | AT91C_US_CHRL_8_BITS
           | AT91C_US_PAR_NONE
           | AT91C_US_NBSTOP_1_BIT
           | AT91C_US_CHMODE_NORMAL;
      USART_Configure(AT91C_BASE_US0, mode, speed, BOARD_MCK);
      AIC_ConfigureIT(AT91C_ID_US0, 0, USARTDriver::defaultISR0);
      AIC_EnableIT(AT91C_ID_US0);
      USART_SetTransmitterEnabled(AT91C_BASE_US0, true);
      USART_SetReceiverEnabled(AT91C_BASE_US0, true);
      USART_ReadBuffer(AT91C_BASE_US0, readBuffer, BUFFER_SIZE);
      ipt = AT91C_US_ENDRX
          | AT91C_US_RXBUFF;
      USART_EnableIt(AT91C_BASE_US0, ipt);
      TRACE_DEBUG("USART0 enabled\n\r");
    break;
    case USART1:
      this->port = USART1;
      TRACE_DEBUG("USART1 is not configured yet\n\r");
    break;
    default:
      TRACE_ERROR("USART%d does not exist\n\r", port);
  }
}

void USARTDriver::setParserFunc(ParserFunc pfunc)
{
  SANITY_CHECK(pfunc);
  parser = pfunc;
}

void USARTDriver::uputchar(char c)
{
  switch(this->port)
  {
    case USART0:
      USART_Write(AT91C_BASE_US0, c, TIMEOUT);
    break;
    case USART1:
      USART_Write(AT91C_BASE_US1, c, TIMEOUT);
    break;
  }
}

void USARTDriver::uprintf(char *str, ...)
{
  unsigned int i = 0;
  va_list arg;
  va_start(arg, str);
  char buffer[255];
  memset(&buffer, 0, sizeof(buffer));
  vsprintf(buffer, str, arg);
  va_end(arg);
  while(buffer[i] != '\0')
  {
    uputchar(buffer[i]);
    i++;
  }
}

void USARTDriver::udmaprintf(char *str, ...)
{
  unsigned int len;
  va_list arg;
  va_start(arg, str);
  char buffer[255];
  memset(&buffer, 0, sizeof(buffer));
  len = vsprintf(buffer, str, arg);
  va_end(arg);
  switch(this->port)
  {
    case USART0:
      USART_WriteBuffer(AT91C_BASE_US0, buffer, len);
    break;
    case USART1:
      USART_WriteBuffer(AT91C_BASE_US1, buffer, len);
    break;
  }
}

unsigned char USARTDriver::ugetchar(void)
{
  switch(this->port)
  {
    case USART0:
      return USART_Read(AT91C_BASE_US0, TIMEOUT);
    break;
    case USART1:
      return USART_Read(AT91C_BASE_US1, TIMEOUT);
    break;
  }
  return 0;
}
