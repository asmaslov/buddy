#include "i2cd.h"
#include "twi.h"
#include "pmc.h"
#include "aic.h"

#include "assert.h"

static I2c *i2c;

static void i2c_handler(void)
{
  SANITY_CHECK(i2c);
  unsigned char status;
  status = TWI_GetMaskedStatus(AT91C_BASE_TWI);
  if(I2C_STATUS_RXRDY(status) && (i2c->transfer.type == TRANSFER_READ))
  {    
    i2c->transfer.data[i2c->transfer.countReal] = TWI_ReadByte(AT91C_BASE_TWI);
    i2c->transfer.countReal++;
    if(i2c->transfer.countReal == (i2c->transfer.countNeed - 1))
    {
      TWI_DisableIt(AT91C_BASE_TWI, AT91C_TWI_RXRDY);
      TWI_Stop(AT91C_BASE_TWI);
      TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_TXCOMP);
    }
  }
  else if(I2C_STATUS_TXRDY(status) && (i2c->transfer.type == TRANSFER_WRITE))
  {
    if (i2c->transfer.countNeed == i2c->transfer.countReal)
    {
      TWI_DisableIt(AT91C_BASE_TWI, AT91C_TWI_TXRDY);
      TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_TXCOMP);
    }
    else
    {
      TWI_WriteByte(AT91C_BASE_TWI, i2c->transfer.data[i2c->transfer.countReal]);
      i2c->transfer.countReal++;
    }
  }
  else if(I2C_STATUS_TXCOMP(status))
  {
    //TRACE_DEBUG("I2C transfer complete\n\r");
    TWI_DisableIt(AT91C_BASE_TWI, AT91C_TWI_TXCOMP);
    if (i2c->transfer.type == TRANSFER_READ)
    {
      i2c->transfer.data[i2c->transfer.countReal] = TWI_ReadByte(AT91C_BASE_TWI);
      i2c->transfer.countReal++;
    }
    SANITY_CHECK(i2c->transfer.async);
    if (i2c->transfer.async->callback != NULL)
    {         
      i2c->transfer.async->callback();
    }
    i2c->transfer.async->status = ASYNC_STATUS_DONE;
  }
}

void i2c_enable(I2c *i)
{
  SANITY_CHECK(i);
  i2c = i;
  i2c->transfer.async->status = ASYNC_STATUS_DONE;
  i2c->iaddress = 0;
  i2c->iaddresslen = 0;
  PIO_Configure(TWI_pins, PIO_LISTSIZE(TWI_pins));    
  PMC_EnablePeripheral(AT91C_ID_TWI);
}

void i2c_disable(void)
{
  PMC_DisablePeripheral(AT91C_ID_TWI);
}

void i2c_configureMaster(unsigned int freq)
{ 
  SANITY_CHECK(freq);
  TWI_ConfigureMaster(AT91C_BASE_TWI, freq, BOARD_MCK);
  AIC_ConfigureIT(AT91C_ID_TWI, AT91C_AIC_PRIOR_LOWEST, i2c_handler);
  AIC_EnableIT(AT91C_ID_TWI);
}

void i2c_setAddress(unsigned char addr)
{
  SANITY_CHECK(i2c);
  i2c->address = addr;
}

 void I2CDriver_setInternalAddress(unsigned int iaddr,
                                   unsigned char iaddrlen)
{
  SANITY_CHECK(i2c);
  i2c->iaddress = iaddr;
  i2c->iaddresslen = iaddrlen;
}

unsigned char i2c_read(unsigned char *data,
                       unsigned int count,
                       Async *async)
{
  SANITY_CHECK(i2c);
  unsigned char status = TRUE;
  //TRACE_DEBUG("I2C read transfer start\n\r");
  SANITY_CHECK((i2c->address & 0x80) == 0);
  SANITY_CHECK((i2c->iaddress & 0xFF000000) == 0);
  SANITY_CHECK(i2c->iaddresslen < 4);
  if(async != NULL)
  {
    i2c->transfer.async = async;
  }
  else
  {
    i2c->transfer.async = &i2c->defaultAsync;
  }
  if(ASYNC_PENDIND(i2c->transfer.async->status))
  {
    TRACE_ERROR("I2C transfer is already pending on trying to read\n\r");
    status = FALSE;
  }
  else
  {
    i2c->transfer.async->status = ASYNC_STATUS_PENDING;
    i2c->transfer.type = TRANSFER_READ;
    i2c->transfer.data = data;
    i2c->transfer.countNeed = count;  
    if(count == 1)
    {
      TWI_Stop(AT91C_BASE_TWI);
      TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_TXCOMP);
    }
    else
    {
      TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_RXRDY);    
    }
    TWI_StartRead(AT91C_BASE_TWI, i2c->address, i2c->iaddress, i2c->iaddresslen);
    i2c->transfer.countReal = 0;
  }
  return status;
}

unsigned char i2c_write(unsigned char *data,
                        unsigned int count,
                        Async *async)
{
  SANITY_CHECK(i2c);
  unsigned char status = TRUE;
  //TRACE_DEBUG("I2C write transfer start\n\r");
  SANITY_CHECK((i2c->address & 0x80) == 0);
  SANITY_CHECK((i2c->iaddress & 0xFF000000) == 0);
  SANITY_CHECK(i2c->iaddresslen < 4);
  if(async != NULL)
  {
    i2c->transfer.async = async;
  }
  else
  {
    i2c->transfer.async = &i2c->defaultAsync;
  }
  if(ASYNC_PENDIND(i2c->transfer.async->status))
  {
    TRACE_ERROR("I2C transfer is already pending on trying to write\n\r");
    status = FALSE;
  }
  else
  {
    i2c->transfer.async->status = ASYNC_STATUS_PENDING;
    i2c->transfer.type = TRANSFER_WRITE;
    i2c->transfer.data = data;
    i2c->transfer.countNeed = count;
    TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_TXRDY);
    TWI_StartWrite(AT91C_BASE_TWI, i2c->address, i2c->iaddress, i2c->iaddresslen, *data);
    i2c->transfer.countReal = 1;
  }
  return status;
}

unsigned char i2c_readNow(unsigned char *data,
                          unsigned int count)
{
  SANITY_CHECK(i2c);
  unsigned char status = TRUE;
  unsigned int attempt;
  TWI_StartRead(AT91C_BASE_TWI, i2c->address, i2c->iaddress, i2c->iaddresslen);
  while (count > 0)
  {
    if (count == 1)
    {
      TWI_Stop(AT91C_BASE_TWI);
    }
    else
    {
      attempt = 0;
      while(!TWI_ByteReceived(AT91C_BASE_TWI) && (++attempt < I2C_MAX_ATTEMPT));
      if (attempt == I2C_MAX_ATTEMPT)
      {
        TRACE_ERROR("I2C byte read timeout\n\r");
        status = FALSE;
      }
      *data++ = TWI_ReadByte(AT91C_BASE_TWI);
      attempt--;
    }
    count--;
  }
  attempt = 0;
  while(!TWI_TransferComplete(AT91C_BASE_TWI) && (++attempt < I2C_MAX_ATTEMPT));
  if (attempt == I2C_MAX_ATTEMPT)
  {
    TRACE_ERROR("I2C transfer complete timeout on trying to read\n\r");
    status = FALSE;
  }
  *data = TWI_ReadByte(AT91C_BASE_TWI);
  return status;
}

unsigned char i2c_writeNow(unsigned char *data,
                           unsigned int count)
{
  SANITY_CHECK(i2c);
  unsigned char status = TRUE;
  unsigned int attempt;
  TWI_StartWrite(AT91C_BASE_TWI, i2c->address, i2c->iaddress, i2c->iaddresslen, *data++);
  count--;
  while (count > 0)
  {
    attempt = 0;
    while(!TWI_ByteSent(AT91C_BASE_TWI) && (++attempt < I2C_MAX_ATTEMPT));
    if (attempt == I2C_MAX_ATTEMPT)
    {
      TRACE_ERROR("I2C byte write timeout\n\r");
      status = FALSE;
    }
    TWI_WriteByte(AT91C_BASE_TWI, *data++);
    count--;
  }
  attempt = 0;
  while(!TWI_TransferComplete(AT91C_BASE_TWI) && (++attempt < I2C_MAX_ATTEMPT));
  if (attempt == I2C_MAX_ATTEMPT)
  {
    TRACE_ERROR("I2C transfer complete timeout on trying to write\n\r");
    status = FALSE;
  }
  return status;
}
