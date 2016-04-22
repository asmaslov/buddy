#ifndef I2CD_H
#define I2CD_H

#include "pio.h"
#include "async.h"

#include <stdio.h>

#define PINS_TWI {0x00000C00, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define I2C_STATUS_TXRDY(status) ((status & AT91C_TWI_TXRDY) == AT91C_TWI_TXRDY)
#define I2C_STATUS_RXRDY(status) ((status & AT91C_TWI_RXRDY) == AT91C_TWI_RXRDY)
#define I2C_STATUS_TXCOMP(status) ((status & AT91C_TWI_TXCOMP) == AT91C_TWI_TXCOMP)

#define DEFAULT_MASTER_ADDRESS 0xFF
#define DEFAULT_MASTER_ADDRESS_LEN 1

#define I2C_MAX_ATTEMPT 50000

static const Pin TWI_pins[] = {
  PINS_TWI  
};

struct Transfer
{
  bool busy;
  unsigned char *transferData;
  unsigned int transferCountNeed;
  unsigned int transferCountReal;
  Async *pTransferAsync;
};

class I2CDriver
{
  private:
    static AT91S_TWI *pI2C;
    static Transfer transfer;
    static void driverHandler(void);
    Async dummyAsync;
  
  public:
    unsigned int iaddress; 
    unsigned char isize;

  public:
    I2CDriver();
    ~I2CDriver();
    void configureMaster(unsigned int frequencyHz);
    // TODO: Slave mode
    //void initSlave(void); 
    void read(unsigned char address,
              unsigned char *data,
              unsigned int count,
              Async *pAsync = NULL);
    void write(unsigned char address,
               unsigned char *data,
               unsigned int count,
               Async *pAsync = NULL);
    void readNow(unsigned char address,
                 unsigned char *data,
                 unsigned int count);
    void writeNow(unsigned char address,
                  unsigned char *data,
                  unsigned int count);
};

#endif //#ifndef I2CD_H
