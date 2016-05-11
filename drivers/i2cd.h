#ifndef I2CD_H
#define I2CD_H

#include "pio.h"
#include "async.h"

#include <stdio.h>

#define PINS_TWI {BIT10 | BIT11, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define I2C_STATUS_TXRDY(status) ((status & AT91C_TWI_TXRDY) == AT91C_TWI_TXRDY)
#define I2C_STATUS_RXRDY(status) ((status & AT91C_TWI_RXRDY) == AT91C_TWI_RXRDY)
#define I2C_STATUS_TXCOMP(status) ((status & AT91C_TWI_TXCOMP) == AT91C_TWI_TXCOMP)

#define DEFAULT_MASTER_ADDRESS 0x0F
#define DEFAULT_MASTER_ADDRESS_LEN 1

//#define I2C_FREQ_HZ 50000
#define I2C_FREQ_HZ 30000
#define I2C_MAX_ATTEMPT 50000

#define TRANSFER_WRITE 0
#define TRANSFER_READ 1

static const Pin TWI_pins[] = { PINS_TWI };

typedef struct _Transfer {
  unsigned char *data;
  unsigned int countNeed;
  unsigned int countReal;
  unsigned char type;
  Async *async;
} Transfer;

typedef struct _I2c {
  Transfer transfer;
  Async defaultAsync;
  unsigned char address;
  unsigned int iaddress; 
  unsigned char iaddresslen;
} I2c;

void i2c_enable(I2c *i);

void i2c_disable(void);

void i2c_configureMaster(unsigned int freq);

void i2c_setAddress(unsigned char addr);

void i2c_setInternalAddress(unsigned int iaddr,
                            unsigned char iaddrlen);

void i2c_read(unsigned char *data,
              unsigned int count,
              Async *async);

void i2c_write(unsigned char *data,
               unsigned int count,
               Async *async);

unsigned char i2c_readNow(unsigned char *data,
                          unsigned int count);

unsigned char i2c_writeNow(unsigned char *data,
                           unsigned int count);

#endif //#ifndef I2CD_H
