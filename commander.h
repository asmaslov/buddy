#ifndef COMMANDER_H
#define COMMANDER_H

#include "comvault.h"
#include "usartd.h"

#define I2C_TRUST_EXCHANGE_PERIODS 20
#define I2C_RETRY_TIMEOUT_PERIODS 10
#define I2C_BUS_DEVICE_MAX_COUNT 3
#define I2C_BUFFER_MAX_SIZE 3
//#define USE_ASYNC_I2C_DRIVER

#define ENDIR12_ADDRESS 0x39
#define ENDIR34_ADDRESS 0x3A

typedef struct {
  unsigned char connected; // bool
  unsigned char disconnected; // bool
  unsigned int id;
  unsigned char dir;
  unsigned char writeBuffer[I2C_BUFFER_MAX_SIZE];
  unsigned char writeBufferSize;
  unsigned char readBuffer[I2C_BUFFER_MAX_SIZE];
  unsigned char readBufferSize;
  unsigned int success;
  unsigned int attepmt;
} I2CNod;

typedef struct {
  unsigned long tick;
  unsigned long compare;
  unsigned int mastertick;
  unsigned int divide;
} SoftwareTimer;

typedef struct _Commander {
  CommandVault *commandVault;
  volatile unsigned char tickerEnabled;
  SoftwareTimer timer;
  I2CNod nods[I2C_BUS_DEVICE_MAX_COUNT];
  unsigned int currentNodIdx;
  unsigned int totalNods;
} Commander;

typedef void (*CommanderTicker)(void);

CommanderTicker commander_init(Commander *c,
                               CommandVault *cv,
                               Comport *cp);

void commander_createNod(unsigned int nodId,
                         unsigned char nodWrtiteBufferSize,
                         unsigned char nodReadBufferSize);

void commander_nodsPowerUp(void);

void commander_nodsPowerDown(void);

#endif //#ifndef COMMANDER_H
