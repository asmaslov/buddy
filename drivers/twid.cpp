#include "twid.h"
#include "twi.h"
#include "board.h"
#include "aic.h"
#include "pio.h"
#include "assert.h"
#include "trace.h"

#include <stdio.h>
#include <string.h>

#define TWITIMEOUTMAX 50000

#if !defined(AT91C_BASE_TWI) && defined(AT91C_BASE_TWI0)
  #define AT91C_BASE_TWI AT91C_BASE_TWI0
  #define AT91C_ID_TWI        AT91C_ID_TWI0
  #define PINS_TWI            PINS_TWI0
#endif
// TWI clock frequency in Hz
#define TWCK            50000

/// Slave address of AT24C chips.
//#define AT24C_ADDRESS   0x50
#define AT24C_ADDRESS   0x01

/// Page size of an AT24C1024 chip (in bytes)
//#define PAGE_SIZE       256
#define PAGE_SIZE       4

// Pio pins to configure.
static const Pin pins[] = {
  PINS_TWI  
};

static Twid twid;

static unsigned char pData[PAGE_SIZE];

void ISR_Twi(void)
{
  TWID_Handler(&twid);
}

void TestCallback()
{
  printf("-I- Callback fired !\n\r");
}

void TWID_Initialize(Twid *pTwid,
                     AT91S_TWI *pTwi)
{
  TRACE_DEBUG("TWID_Initialize()\n\r");
  SANITY_CHECK(pTwid);
  SANITY_CHECK(pTwi);
  pTwid->pTwi = pTwi;
  pTwid->pTransfer = 0;
}

void TWID_Handler(Twid *pTwid)
{
  unsigned char status;
  AsyncTwi *pTransfer = (AsyncTwi *) pTwid->pTransfer;
  AT91S_TWI *pTwi = pTwid->pTwi;
  SANITY_CHECK(pTwid);
  // Retrieve interrupt status
  status = TWI_GetMaskedStatus(pTwi);
  // Byte received
  if (TWI_STATUS_RXRDY(status))
  {
    pTransfer->pData[pTransfer->transferred] = TWI_ReadByte(pTwi);
    pTransfer->transferred++;
    // Transfer finished ?
    if (pTransfer->transferred == pTransfer->num)
    {
      TWI_DisableIt(pTwi, AT91C_TWI_RXRDY);
      TWI_EnableIt(pTwi, AT91C_TWI_TXCOMP);
    }
    // Last byte ?
    else if (pTransfer->transferred == (pTransfer->num - 1))
    {
      TWI_Stop(pTwi);
    }
  }
  // Byte sent
  else if (TWI_STATUS_TXRDY(status))
  {
    // Transfer finished ?
    if (pTransfer->transferred == pTransfer->num)
    {
      TWI_DisableIt(pTwi, AT91C_TWI_TXRDY);
      TWI_EnableIt(pTwi, AT91C_TWI_TXCOMP);
    }
    // Bytes remaining
    else
    {
      TWI_WriteByte(pTwi, pTransfer->pData[pTransfer->transferred]);
      pTransfer->transferred++;
    }
  }
  // Transfer complete
  else if (TWI_STATUS_TXCOMP(status))
  {
    TWI_DisableIt(pTwi, AT91C_TWI_TXCOMP);
    pTransfer->status = 0;
    if (pTransfer->callback)
    {         
      pTransfer->callback((Async *) pTransfer);
    }
    pTwid->pTransfer = 0;
  }
}

unsigned char TWID_Read(Twid *pTwid,
                        unsigned char address,
                        unsigned int iaddress,
                        unsigned char isize,
                        unsigned char *pData,
                        unsigned int num,
                        Async *pAsync)
{
  AT91S_TWI *pTwi = pTwid->pTwi;
  AsyncTwi *pTransfer = (AsyncTwi *) pTwid->pTransfer;
  unsigned int timeout;
  TRACE_DEBUG("TWID_Read()\n\r");
  SANITY_CHECK(pTwid);
  SANITY_CHECK((address & 0x80) == 0);
  SANITY_CHECK((iaddress & 0xFF000000) == 0);
  SANITY_CHECK(isize < 4);
  // Check that no transfer is already pending
  if (pTransfer)
  {
    TRACE_ERROR("TWID_Read: A transfer is already pending\n\r");
    return TWID_ERROR_BUSY;
  }
  // Set STOP signal if only one byte is sent
  if (num == 1)
  {
    TWI_Stop(pTwi);
  }
  // Asynchronous transfer
  if (pAsync)
  {
    // Update the transfer descriptor
    pTwid->pTransfer = pAsync;
    pTransfer = (AsyncTwi *) pAsync;
    pTransfer->status = ASYNC_STATUS_PENDING;
    pTransfer->pData = pData;
    pTransfer->num = num;
    pTransfer->transferred = 0;
    // Enable read interrupt and start the transfer
    TWI_EnableIt(pTwi, AT91C_TWI_RXRDY);
    TWI_StartRead(pTwi, address, iaddress, isize);
  }
  // Synchronous transfer
  else
  {
    // Start read
    TWI_StartRead(pTwi, address, iaddress, isize);
    // Read all bytes, setting STOP before the last byte
    while (num > 0)
    {
      // Last byte
      if (num == 1)
      {
        TWI_Stop(pTwi);
      }
      // Wait for byte then read and store it
      timeout = 0;
      while( !TWI_ByteReceived(pTwi) && (++timeout<TWITIMEOUTMAX) );
      if (timeout == TWITIMEOUTMAX)
      {
        TRACE_ERROR("TWID Timeout BR\n\r");
      }
      *pData++ = TWI_ReadByte(pTwi);
      num--;
    }
    // Wait for transfer to be complete
    timeout = 0;
    while( !TWI_TransferComplete(pTwi) && (++timeout<TWITIMEOUTMAX) );
    if (timeout == TWITIMEOUTMAX)
    {
      TRACE_ERROR("TWID Timeout TC\n\r");
    }
  }
  return 0;
}

unsigned char TWID_Write(Twid *pTwid,
                         unsigned char address,
                         unsigned int iaddress,
                         unsigned char isize,
                         unsigned char *pData,
                         unsigned int num,
                         Async *pAsync)
{
  AT91S_TWI *pTwi = pTwid->pTwi;
  AsyncTwi *pTransfer = (AsyncTwi *) pTwid->pTransfer;
  unsigned int timeout;
  TRACE_DEBUG("TWID_Write()\n\r");
  TRACE_DEBUG("0x%X\n\r", pData[0]);
  SANITY_CHECK(pTwi);
  SANITY_CHECK((address & 0x80) == 0);
  SANITY_CHECK((iaddress & 0xFF000000) == 0);
  SANITY_CHECK(isize < 4);
  // Check that no transfer is already pending
  if (pTransfer)
  {
    TRACE_ERROR("TWI_Write: A transfer is already pending\n\r");
    return TWID_ERROR_BUSY;
  }
  // Asynchronous transfer
  if (pAsync)
  {
    // Update the transfer descriptor
    pTwid->pTransfer = pAsync;
    pTransfer = (AsyncTwi *) pAsync;
    pTransfer->status = ASYNC_STATUS_PENDING;
    pTransfer->pData = pData;
    pTransfer->num = num;
    pTransfer->transferred = 1;
    // Enable write interrupt and start the transfer
    TWI_StartWrite(pTwi, address, iaddress, isize, *pData);
    TWI_EnableIt(pTwi, AT91C_TWI_TXRDY);
  }
  // Synchronous transfer
  else
  {
    // Start write
    TWI_StartWrite(pTwi, address, iaddress, isize, *pData++);
    num--;
    // Send all bytes
    while (num > 0)
    {
      // Wait before sending the next byte
      timeout = 0;
      while( !TWI_ByteSent(pTwi) && (++timeout<TWITIMEOUTMAX) );
      if (timeout == TWITIMEOUTMAX)
      {
        TRACE_ERROR("TWID Timeout BS\n\r");
      }
      TWI_WriteByte(pTwi, *pData++);
      num--;
    }
    // Wait for actual end of transfer
    timeout = 0;
    while( !TWI_TransferComplete(pTwi) && (++timeout<TWITIMEOUTMAX) );
    if (timeout == TWITIMEOUTMAX)
    {
      TRACE_ERROR("TWID Timeout TC2\n\r");
    }
  }
  return 0;
}

void initTWID(void)
{
  unsigned int i;
  Async async;
  unsigned int numErrors;
  
  PIO_Configure(pins, PIO_LISTSIZE(pins));
  // Configure TWI
  // In IRQ mode: to avoid problems, the priority of the TWI IRQ must be max
  // In polling mode: try to disable all IRQs if possible
  // (in this example it does not matter, there is only the TWI IRQ active)
  
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_TWI;
  TWI_ConfigureMaster(AT91C_BASE_TWI, TWCK, BOARD_MCK);
  TWID_Initialize(&twid, AT91C_BASE_TWI);
  AIC_ConfigureIT(AT91C_ID_TWI, 0, ISR_Twi);
  AIC_EnableIT(AT91C_ID_TWI);
  
  // Erase page #0 and #1
  memset(pData, 0, PAGE_SIZE);
  printf("-I- Filling page #0 with zeroes ...\n\r");
  TWID_Write(&twid, AT24C_ADDRESS, 0x0000, 2, pData, PAGE_SIZE, 0);
  // Wait at least 10 ms
  // TODO: Make honest delay_ms(10)
  for (i=0; i < 1000000; i++);
  printf("-I- Filling page #1 with zeroes ...\n\r");
  TWID_Write(&twid, AT24C_ADDRESS, 0x0100, 2, pData, PAGE_SIZE, 0);
  // Wait at least 10 ms
  // TODO: Make honest delay_ms(10)
  for (i=0; i < 1000000; i++);
  // Synchronous operation
  printf("-I- Read/write on page #0 (polling mode)\n\r");
  // Write checkerboard pattern in first page
  for (i=0; i < PAGE_SIZE; i++)
  {
    // Even
    if ((i & 1) == 0)
    {
      pData[i] = 0xA5;
    }
    // Odd
    else
    {
      pData[i] = 0x5A;
    }
  }
  TWID_Write(&twid, AT24C_ADDRESS, 0x0000, 2, pData, PAGE_SIZE, 0);
  // Wait at least 10 ms
  // TODO: Make honest delay_ms(10)
  for (i=0; i < 1000000; i++);
  // Read back data
  memset(pData, 0, PAGE_SIZE);
  TWID_Read(&twid, AT24C_ADDRESS, 0x0000, 2, pData, PAGE_SIZE, 0);
  // Compare
  numErrors = 0;
  for (i=0; i < PAGE_SIZE; i++)
  {
    // Even
    if (((i & 1) == 0) && (pData[i] != 0xA5))
    {
      printf("-E- Data mismatch at offset #%u: expected 0xA5, read 0x%02X\n\r", i, pData[i]);
      numErrors++;
    }
    // Odd
    else if (((i & 1) == 1) && (pData[i] != 0x5A))
    {
      printf("-E- Data mismatch at offset #%u: expected 0x5A, read 0x%02X\n\r", i, pData[i]);
      numErrors++;
    }
  }
  printf("-I- %u comparison error(s) found\n\r", numErrors);
  // Asynchronous operation
  printf("-I- Read/write on page #1 (IRQ mode)\n\r");
  // Write checkerboard pattern in first page
  for (i=0; i < PAGE_SIZE; i++)
  {
    // Even
    if ((i & 1) == 0)
    {
      pData[i] = 0xA5;
    }
    // Odd
    else
    {
      pData[i] = 0x5A;
    }
  }
  memset(&async, 0, sizeof(async));
  async.callback = (void *) TestCallback;
  TWID_Write(&twid, AT24C_ADDRESS, 0x0100, 2, pData, PAGE_SIZE, &async);
  while (!ASYNC_IsFinished(&async));
  // Wait at least 10 ms
  // TODO: Make honest delay_ms(10)
  for (i=0; i < 1000000; i++);
  // Read back data
  memset(pData, 0, PAGE_SIZE);
  memset(&async, 0, sizeof(async));
  async.callback = (void *) TestCallback;
  TWID_Read(&twid, AT24C_ADDRESS, 0x0100, 2, pData, PAGE_SIZE, &async);
  while (!ASYNC_IsFinished(&async));
  // Compare
  numErrors = 0;
  for (i=0; i < PAGE_SIZE; i++)
  {
    // Even
    if (((i & 1) == 0) && (pData[i] != 0xA5))
    {
      printf("-E- Data mismatch at offset #%u: expected 0xA5, read 0x%02X\n\r", i, pData[i]);
      numErrors++;
    }
    // Odd
    else if (((i & 1) == 1) && (pData[i] != 0x5A))
    {
      printf("-E- Data mismatch at offset #%u: expected 0x5A, read 0x%02X\n\r", i, pData[i]);
      numErrors++;
    }
  }
  printf("-I- %u comparison error(s) found\n\r", numErrors);
}
