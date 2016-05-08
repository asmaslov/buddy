#include "i2cd.h"
#include "twi.h"
#include "pmc.h"
#include "aic.h"

#include "assert.h"

I2c *i2cLocal;

static void i2c_handler(void)
{
  SANITY_CHECK(i2cLocal);
  unsigned char status;
  status = TWI_GetMaskedStatus(AT91C_BASE_TWI);
  if(I2C_STATUS_RXRDY(status) && (i2cLocal->transfer.type == TRANSFER_READ))
  {    
    i2cLocal->transfer.data[i2cLocal->transfer.countReal] = TWI_ReadByte(AT91C_BASE_TWI);
    i2cLocal->transfer.countReal++;
    if(i2cLocal->transfer.countReal == (i2cLocal->transfer.countNeed - 1))
    {
      TWI_DisableIt(AT91C_BASE_TWI, AT91C_TWI_RXRDY);
      TWI_Stop(AT91C_BASE_TWI);
      TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_TXCOMP);
    }
  }
  else if(I2C_STATUS_TXRDY(status) && (i2cLocal->transfer.type == TRANSFER_WRITE))
  {
    if (i2cLocal->transfer.countNeed == i2cLocal->transfer.countReal)
    {
      TWI_DisableIt(AT91C_BASE_TWI, AT91C_TWI_TXRDY);
      TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_TXCOMP);
    }
    else
    {
      TWI_WriteByte(AT91C_BASE_TWI, i2cLocal->transfer.data[i2cLocal->transfer.countReal]);
      i2cLocal->transfer.countReal++;
    }
  }
  else if(I2C_STATUS_TXCOMP(status))
  {
    //TRACE_DEBUG("I2C transfer complete\n\r");
    TWI_DisableIt(AT91C_BASE_TWI, AT91C_TWI_TXCOMP);
    if (i2cLocal->transfer.type == TRANSFER_READ)
    {
      i2cLocal->transfer.data[i2cLocal->transfer.countReal] = TWI_ReadByte(AT91C_BASE_TWI);
      i2cLocal->transfer.countReal++;
    }
    SANITY_CHECK(i2cLocal->transfer.async);
    if (i2cLocal->transfer.async->callback != NULL)
    {         
      i2cLocal->transfer.async->callback();
    }
    i2cLocal->transfer.async->status = ASYNC_STATUS_DONE;
  }
}

void i2c_enable(I2c *i)
{
  SANITY_CHECK(i);
  i2cLocal = i;
  i2cLocal->transfer.async->status = ASYNC_STATUS_DONE;
  i2cLocal->iaddress = 0;
  i2cLocal->iaddresslen = 0;
  PIO_Configure(TWI_pins, PIO_LISTSIZE(TWI_pins));
  
  // Magic here
  AT91C_BASE_PIOA->PIO_MDER = BIT10 | BIT11;
  AT91C_BASE_PIOA->PIO_PPUDR = BIT10 | BIT11;
    
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
  SANITY_CHECK(i2cLocal);
  i2cLocal->address = addr;
}

 void I2CDriver_setInternalAddress(unsigned int iaddr,
                                   unsigned char iaddrlen)
{
  SANITY_CHECK(i2cLocal);
  i2cLocal->iaddress = iaddr;
  i2cLocal->iaddresslen = iaddrlen;
}

void i2c_read(unsigned char *data,
              unsigned int count,
              Async *async)
{
  SANITY_CHECK(i2cLocal);
  //TRACE_DEBUG("I2C read transfer start\n\r");
  SANITY_CHECK((i2cLocal->address & 0x80) == 0);
  SANITY_CHECK((i2cLocal->iaddress & 0xFF000000) == 0);
  SANITY_CHECK(i2cLocal->iaddresslen < 4);
  if(async != NULL)
  {
    i2cLocal->transfer.async = async;
  }
  else
  {
    i2cLocal->transfer.async = &i2cLocal->defaultAsync;
  }
  if(ASYNC_PENDIND(i2cLocal->transfer.async->status))
  {
    TRACE_ERROR("I2C transfer is already pending on trying to read\n\r");
  }
  else
  {
    i2cLocal->transfer.async->status = ASYNC_STATUS_PENDING;
    i2cLocal->transfer.type = TRANSFER_READ;
    i2cLocal->transfer.data = data;
    i2cLocal->transfer.countNeed = count;  
    if(count == 1)
    {
      TWI_Stop(AT91C_BASE_TWI);
      TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_TXCOMP);
    }
    else
    {
      TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_RXRDY);    
    }
    TWI_StartRead(AT91C_BASE_TWI, i2cLocal->address, i2cLocal->iaddress, i2cLocal->iaddresslen);
    i2cLocal->transfer.countReal = 0;
  }
}

void i2c_write(unsigned char *data,
               unsigned int count,
               Async *async)
{
  SANITY_CHECK(i2cLocal);
  //TRACE_DEBUG("I2C write transfer start\n\r");
  SANITY_CHECK((i2cLocal->address & 0x80) == 0);
  SANITY_CHECK((i2cLocal->iaddress & 0xFF000000) == 0);
  SANITY_CHECK(i2cLocal->iaddresslen < 4);
  if(async != NULL)
  {
    i2cLocal->transfer.async = async;
  }
  else
  {
    i2cLocal->transfer.async = &i2cLocal->defaultAsync;
  }
  if(ASYNC_PENDIND(i2cLocal->transfer.async->status))
  {
    TRACE_ERROR("I2C transfer is already pending on trying to write\n\r");
  }
  else
  {
    i2cLocal->transfer.async->status = ASYNC_STATUS_PENDING;
    i2cLocal->transfer.type = TRANSFER_WRITE;
    i2cLocal->transfer.data = data;
    i2cLocal->transfer.countNeed = count;
    TWI_EnableIt(AT91C_BASE_TWI, AT91C_TWI_TXRDY);
    TWI_StartWrite(AT91C_BASE_TWI, i2cLocal->address, i2cLocal->iaddress, i2cLocal->iaddresslen, *data);
    i2cLocal->transfer.countReal = 1;
  }
}

unsigned char i2c_readNow(unsigned char *data,
                          unsigned int count)
{
  SANITY_CHECK(i2cLocal);
  unsigned char status = TRUE;
  unsigned int attempt;
  TWI_StartRead(AT91C_BASE_TWI, i2cLocal->address, i2cLocal->iaddress, i2cLocal->iaddresslen);
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
  SANITY_CHECK(i2cLocal);
  unsigned char status = TRUE;
  unsigned int attempt;
  TWI_StartWrite(AT91C_BASE_TWI, i2cLocal->address, i2cLocal->iaddress, i2cLocal->iaddresslen, *data++);
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
  if (attempt < I2C_MAX_ATTEMPT)
  {
    TRACE_ERROR("I2C transfer complete timeout on trying to write\n\r");
    status = FALSE;
  }
  return status;
}
