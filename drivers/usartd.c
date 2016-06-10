#include "usartd.h"
#include "usart.h"
#include "pmc.h"
#include "aic.h"

#include "assert.h"

#include <stdarg.h>
#include <string.h>

static Comport *comport;

static void comport_handler0(void)
{
  SANITY_CHECK(comport);
  AIC_ClearIT(AT91C_ID_US0);
  unsigned int status;
  status = AT91C_BASE_US0->US_CSR;
  if((status & AT91C_US_ENDRX) == AT91C_US_ENDRX)
  {
    TRACE_DEBUG("USART0 read buffer full\n\r");
    if(comport->parser)
    {
      comport->parser(comport->readBuffer, USART_BUFFER_SIZE);
    }
    if(!USART_ReadBuffer(AT91C_BASE_US0, comport->readBuffer, USART_BUFFER_SIZE))
    {
      TRACE_ERROR("USART0 buffers busy\n\r");
    }
  }
}

static void comport_handler1(void)
{
  SANITY_CHECK(comport);
}

void comport_enable(Comport *cp)
{
  SANITY_CHECK(cp);
  comport = cp;
  PIO_Configure(USART0_pins, PIO_LISTSIZE(USART0_pins)); 
  PMC_EnablePeripheral(AT91C_ID_US0);
}

void comport_disable(void)
{
  AIC_DisableIT(AT91C_ID_US0);
  AIC_DisableIT(AT91C_ID_US1);
  PMC_DisablePeripheral(AT91C_ID_US0);
}

void comport_configure(unsigned char portnum,
                       unsigned int speed)
{
  SANITY_CHECK(comport);
  unsigned int mode, ipt;
  switch(portnum)
  {
    case USART0:
      comport->port = USART0;
      mode = AT91C_US_USMODE_NORMAL
           | AT91C_US_CLKS_CLOCK
           | AT91C_US_CHRL_8_BITS
           | AT91C_US_PAR_NONE
           | AT91C_US_NBSTOP_1_BIT
           | AT91C_US_CHMODE_NORMAL;
      USART_Configure(AT91C_BASE_US0, mode, speed, BOARD_MCK);
      AIC_ConfigureIT(AT91C_ID_US0, AT91C_AIC_PRIOR_LOWEST, comport_handler0);
      AIC_EnableIT(AT91C_ID_US0);
      USART_SetTransmitterEnabled(AT91C_BASE_US0, TRUE);
      USART_SetReceiverEnabled(AT91C_BASE_US0, TRUE);
      USART_ReadBuffer(AT91C_BASE_US0, comport->readBuffer, USART_BUFFER_SIZE);
      ipt = AT91C_US_ENDRX   // One of the buffers is full
          | AT91C_US_RXBUFF; // Both buffers are full
      USART_EnableIt(AT91C_BASE_US0, ipt);
      TRACE_DEBUG("USART0 enabled\n\r");
    break;
    case USART1:
      comport->port = USART1;
      TRACE_DEBUG("USART1 is not configured yet\n\r");
    break;
    default:
      TRACE_ERROR("USART%d does not exist\n\r", portnum);
  }
}

void comport_setParserFunc(ParserFunc pfunc)
{
  SANITY_CHECK(comport);
  SANITY_CHECK(pfunc);
  comport->parser = pfunc;
}

void comport_uputchar(char c)
{
  SANITY_CHECK(comport);
  switch(comport->port)
  {
    case USART0:
      USART_Write(AT91C_BASE_US0, c, USART_TIMEOUT);
    break;
    case USART1:
      USART_Write(AT91C_BASE_US1, c, USART_TIMEOUT);
    break;
  }
}

void comport_uputs(char *str, int size)
{
  for(int i = 0; i < size; i++)
  {
    comport_uputchar(*(str + i));
  }
}

void comport_uprintf(char *str, ...)
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
    comport_uputchar(buffer[i]);
    i++;
  }
}

void comport_udmaputs(char *str, int size)
{
  SANITY_CHECK(comport);
  switch(comport->port)
  {
    case USART0:
      USART_WriteBuffer(AT91C_BASE_US0, str, size);
    break;
    case USART1:
      USART_WriteBuffer(AT91C_BASE_US1, str, size);
    break;
  }
}

void comport_udmaprintf(char *str, ...)
{
  SANITY_CHECK(comport);
  unsigned int len;
  va_list arg;
  va_start(arg, str);
  char buffer[255];
  memset(&buffer, 0, sizeof(buffer));
  len = vsprintf(buffer, str, arg);
  va_end(arg);
  switch(comport->port)
  {
    case USART0:
      USART_WriteBuffer(AT91C_BASE_US0, buffer, len);
    break;
    case USART1:
      USART_WriteBuffer(AT91C_BASE_US1, buffer, len);
    break;
  }
}

unsigned char comport_ugetchar(void)
{
  SANITY_CHECK(comport);
  switch(comport->port)
  {
    case USART0:
      return USART_Read(AT91C_BASE_US0, USART_TIMEOUT);
    break;
    case USART1:
      return USART_Read(AT91C_BASE_US1, USART_TIMEOUT);
    break;
  }
  return 0;
}

void comport_uread(void)
{
  SANITY_CHECK(comport);
  switch(comport->port)
  {
    case USART0:
      USART_ClearReadBuffers(AT91C_BASE_US0);
      USART_ReadBuffer(AT91C_BASE_US0, comport->readBuffer, USART_BUFFER_SIZE);
    break;
    case USART1:
      USART_ClearReadBuffers(AT91C_BASE_US1);
      USART_ReadBuffer(AT91C_BASE_US1, comport->readBuffer, USART_BUFFER_SIZE);
    break;
  }
}
