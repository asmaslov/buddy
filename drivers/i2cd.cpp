#include "i2cd.h"
#include "twi.h"
#include "aic.h"
#include "assert.h"

AT91S_TWI *I2CDriver::pI2C;
Transfer I2CDriver::transfer;

void dummyCallback()
{
  TRACE_DEBUG("Dummy callback fired !\n\r");
}

I2CDriver::I2CDriver()
{
  dummyAsync.callback = dummyCallback;
  dummyAsync.setDone();
  transfer.busy = false;
  iaddress = DEFAULT_MASTER_ADDRESS;
  isize = DEFAULT_MASTER_ADDRESS_LEN;
  pI2C = AT91C_BASE_TWI;
  PIO_Configure(TWI_pins, PIO_LISTSIZE(TWI_pins));
}

I2CDriver::~I2CDriver()
{

}

void I2CDriver::configureMaster(unsigned int frequencyHz)
{ 
  SANITY_CHECK(frequencyHz);
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_TWI;
  TWI_ConfigureMaster(AT91C_BASE_TWI, frequencyHz, BOARD_MCK);
  AIC_ConfigureIT(AT91C_ID_TWI, 0, I2CDriver::driverHandler);
  AIC_EnableIT(AT91C_ID_TWI);
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
    TWI_DisableIt(pI2C, AT91C_TWI_TXCOMP);
    transfer.pTransferAsync->setDone();
    if (transfer.pTransferAsync->callback)
    {         
      transfer.pTransferAsync->callback();
    }
    transfer.busy = false;
  }
}

void I2CDriver::read(unsigned char address,
                     unsigned char *data,
                     unsigned int count,
                     Async *pAsync)
{
  TRACE_DEBUG("I2C read transfer start\n\r");
  SANITY_CHECK(pI2C);
  SANITY_CHECK((address & 0x80) == 0);
  SANITY_CHECK((iaddress & 0xFF000000) == 0);
  SANITY_CHECK(isize < 4);
  if (transfer.busy == true)
  {
    TRACE_ERROR("I2C transfer is already pending\\n\r");
  }
  else
  {
    if (count == 1)
    {
      TWI_Stop(pI2C);
    }
    transfer.busy = true;
    transfer.transferData = data;
    transfer.transferCountNeed = count;  
    if(pAsync != NULL)
    {
      pAsync->setPending();
      transfer.pTransferAsync = pAsync;
    }
    else
    {
      dummyAsync.setPending();
      transfer.pTransferAsync = &dummyAsync;
    }
    TWI_EnableIt(pI2C, AT91C_TWI_RXRDY);
    TWI_StartRead(pI2C, address, iaddress, isize);
    transfer.transferCountReal = 1;
  }
}

void I2CDriver::write(unsigned char address,
                      unsigned char *data,
                      unsigned int count,
                      Async *pAsync)
{
  TRACE_DEBUG("I2C write transfer start\n\r");
  TRACE_DEBUG("0x%X\n\r", data[0]);  
  SANITY_CHECK(pI2C);
  SANITY_CHECK((address & 0x80) == 0);
  SANITY_CHECK((iaddress & 0xFF000000) == 0);
  SANITY_CHECK(isize < 4);
  if (transfer.busy == true)
  {
    TRACE_ERROR("I2C transfer is already pending\n\r");
  }
  else
  {
    transfer.busy = true;
    transfer.transferData = data;
    transfer.transferCountNeed = count;
    if(pAsync != NULL)
    {
      pAsync->setPending();
      transfer.pTransferAsync = pAsync;
    }
    else
    {
      dummyAsync.setPending();
      transfer.pTransferAsync = &dummyAsync;
    }
    TWI_EnableIt(pI2C, AT91C_TWI_TXRDY);
    TWI_StartWrite(pI2C, address, iaddress, isize, *data);
    transfer.transferCountReal = 1;
  }
}

void I2CDriver::readNow(unsigned char address,
                        unsigned char *data,
                        unsigned int count)
{
  unsigned int attempt;
  TWI_StartRead(pI2C, address, iaddress, isize);
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

void I2CDriver::writeNow(unsigned char address,
                         unsigned char *data,
                         unsigned int count)
{
  unsigned int attempt;
  TWI_StartWrite(pI2C, address, iaddress, isize, *data++);
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
