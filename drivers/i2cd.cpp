#include "i2cd.h"
#include "twi.h"
#include "pmc.h"
#include "aic.h"
#include "assert.h"

AT91S_TWI *I2CDriver::pI2C;
Transfer I2CDriver::transfer;
Async I2CDriver::defaultAsync;

I2CDriver::I2CDriver()
{
  transfer.async->setDone();
  iaddress = 0;
  iaddresslen = 0;
  pI2C = AT91C_BASE_TWI;
  PIO_Configure(TWI_pins, PIO_LISTSIZE(TWI_pins));
  PMC_EnablePeripheral(AT91C_ID_TWI);
}

I2CDriver::~I2CDriver()
{
  PMC_DisablePeripheral(AT91C_ID_TWI);
}

void I2CDriver::configureMaster(void)
{ 
  TWI_ConfigureMaster(AT91C_BASE_TWI, I2C_FREQ_HZ, BOARD_MCK);
  AIC_ConfigureIT(AT91C_ID_TWI, 0, I2CDriver::driverHandler);
  AIC_EnableIT(AT91C_ID_TWI);
}

void I2CDriver::setAddress(unsigned char addr)
{
  address = addr;
}

 void I2CDriver::setInternalAddress(unsigned int iaddr,
                                    unsigned char iaddrlen)
{
  iaddress = iaddr;
  iaddresslen = iaddrlen;
}

void I2CDriver::driverHandler(void)
{
  unsigned char status;
  status = TWI_GetMaskedStatus(pI2C);
  if (I2C_STATUS_RXRDY(status))
  {    
    transfer.transferData[transfer.transferCountReal] = TWI_ReadByte(pI2C);
    transfer.transferCountReal++;
    if (transfer.transferCountNeed == transfer.transferCountReal)
    {
      TWI_DisableIt(pI2C, AT91C_TWI_RXRDY);
      TWI_EnableIt(pI2C, AT91C_TWI_TXCOMP);
    }
    else if (transfer.transferCountReal == (transfer.transferCountNeed - 1))
    {
      TWI_Stop(pI2C);
    }
  }
  else if (I2C_STATUS_TXRDY(status))
  {
    if (transfer.transferCountNeed == transfer.transferCountReal)
    {
      TWI_DisableIt(pI2C, AT91C_TWI_TXRDY);
      TWI_EnableIt(pI2C, AT91C_TWI_TXCOMP);
    }
    else
    {
      TWI_WriteByte(pI2C, transfer.transferData[transfer.transferCountReal]);
      transfer.transferCountReal++;
    }
  }
  else if (I2C_STATUS_TXCOMP(status))
  {
    TRACE_DEBUG("I2C transfer complete\n\r");
    TWI_DisableIt(pI2C, AT91C_TWI_TXCOMP);
    SANITY_CHECK(transfer.async);
    transfer.async->setDone();
    if (transfer.async->callback != NULL)
    {         
      transfer.async->callback();
    }
    transfer.async->setDone();
  }
}

void I2CDriver::read(unsigned char *data,
                     unsigned int count,
                     Async *async)
{
  TRACE_DEBUG("I2C read transfer start\n\r");
  SANITY_CHECK(pI2C);
  SANITY_CHECK((address & 0x80) == 0);
  SANITY_CHECK((iaddress & 0xFF000000) == 0);
  SANITY_CHECK(iaddresslen < 4);
  if(async != NULL)
  {
    transfer.async = async;
  }
  else
  {
    transfer.async = &I2CDriver::defaultAsync;
  }
  if (transfer.async->busy())
  {
    TRACE_ERROR("I2C transfer is already pending\n\r");
  }
  else
  {
    if (count == 1)
    {
      TWI_Stop(pI2C);
    }
    transfer.async->setPending();
    transfer.transferData = data;
    transfer.transferCountNeed = count;  
    TWI_EnableIt(pI2C, AT91C_TWI_RXRDY);
    TWI_StartRead(pI2C, address, iaddress, iaddresslen);
    transfer.transferCountReal = 1;
  }
}

void I2CDriver::write(unsigned char *data,
                      unsigned int count,
                      Async *async)
{
  TRACE_DEBUG("I2C write transfer start\n\r");
  SANITY_CHECK(pI2C);
  SANITY_CHECK((address & 0x80) == 0);
  SANITY_CHECK((iaddress & 0xFF000000) == 0);
  SANITY_CHECK(iaddresslen < 4);
  if(async != NULL)
  {
    transfer.async = async;
  }
  else
  {
    transfer.async = &I2CDriver::defaultAsync;
  }
  if (transfer.async->busy())
  {
    TRACE_ERROR("I2C transfer is already pending\n\r");
  }
  else
  {
    transfer.async->setPending();
    transfer.transferData = data;
    transfer.transferCountNeed = count;
    TWI_EnableIt(pI2C, AT91C_TWI_TXRDY);
    TWI_StartWrite(pI2C, address, iaddress, iaddresslen, *data);
    transfer.transferCountReal = 1;
  }
}

void I2CDriver::readNow(unsigned char *data,
                        unsigned int count)
{
  unsigned int attempt;
  TWI_StartRead(pI2C, address, iaddress, iaddresslen);
  while (count > 0)
  {
    if (count == 1)
    {
      TWI_Stop(pI2C);
    }
    attempt = 0;
    while(!TWI_ByteReceived(pI2C) && (++attempt < I2C_MAX_ATTEMPT) );
    if (attempt == I2C_MAX_ATTEMPT)
    {
      TRACE_ERROR("I2C byte read timeout\n\r");
    }
    *data++ = TWI_ReadByte(pI2C);
    attempt--;
  }
  attempt = 0;
  while(!TWI_TransferComplete(pI2C) && (++attempt < I2C_MAX_ATTEMPT) );
  if (attempt == I2C_MAX_ATTEMPT)
  {
    TRACE_ERROR("I2C transfer complete timeout\n\r");
  }
}

void I2CDriver::writeNow(unsigned char *data,
                         unsigned int count)
{
  unsigned int attempt;
  TWI_StartWrite(pI2C, address, iaddress, iaddresslen, *data++);
  count--;
  while (count > 0)
  {
    attempt = 0;
    while(!TWI_ByteSent(pI2C) && (++attempt < I2C_MAX_ATTEMPT));
    if (attempt == I2C_MAX_ATTEMPT)
    {
      TRACE_ERROR("I2C byte send timeout\n\r");
    }
    TWI_WriteByte(pI2C, *data++);
    count--;
  }
  attempt = 0;
  while(!TWI_TransferComplete(pI2C) && (++attempt < I2C_MAX_ATTEMPT));
  if (attempt < I2C_MAX_ATTEMPT)
  {
    TRACE_ERROR("I2C transfer complete timeout\n\r");
  }
}
