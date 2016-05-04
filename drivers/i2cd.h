#ifndef I2CD_H
#define I2CD_H

#include "pio.h"
#include "async.h"

#include <stdio.h>

#define PINS_TWI {0x00000C00, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define I2C_STATUS_TXRDY(status) ((status & AT91C_TWI_TXRDY) == AT91C_TWI_TXRDY)
#define I2C_STATUS_RXRDY(status) ((status & AT91C_TWI_RXRDY) == AT91C_TWI_RXRDY)
#define I2C_STATUS_TXCOMP(status) ((status & AT91C_TWI_TXCOMP) == AT91C_TWI_TXCOMP)

#define DEFAULT_MASTER_ADDRESS 0x0F
#define DEFAULT_MASTER_ADDRESS_LEN 1

#define I2C_FREQ_HZ 10000
#define I2C_BUFFER_SIZE 3
#define I2C_MAX_ATTEMPT 50000

#define TRANSFER_WRITE 0
#define TRANSFER_READ 1

typedef struct {
  unsigned int id;
  unsigned char wrtiteBuffer[I2C_BUFFER_SIZE];
  unsigned char readBuffer[I2C_BUFFER_SIZE];
} I2CNod;

static const Pin TWI_pins[] = { PINS_TWI };

struct Transfer
{
  unsigned char *transferData;
  unsigned int transferCountNeed;
  unsigned int transferCountReal;
  unsigned char transferType;
  Async *async;
};

typedef void (*I2CReceiveFunc)(unsigned char *buf, int size);

class I2CDriver
{
  private:
    static AT91S_TWI *pI2C;
    static Transfer transfer;
    static void driverHandler(void);
    static Async defaultAsync;
    unsigned char address;
    unsigned int iaddress; 
    unsigned char iaddresslen;

  public:
    I2CDriver();
    ~I2CDriver();
    void configureMaster(void);
    // TODO: Slave mode
    //void initSlave(void); 
    void setAddress(unsigned char addr);
    void setInternalAddress(unsigned int iaddr,
                            unsigned char iaddrlen);
    void read(unsigned char *data,
              unsigned int count,
              Async *async = NULL);
    void write(unsigned char *data,
               unsigned int count,
               Async *async = NULL);
    void readNow(unsigned char *data,
                 unsigned int count);
    void writeNow(unsigned char *data,
                  unsigned int count);
};

#endif //#ifndef I2CD_H
