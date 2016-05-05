#ifndef COMMANDER_H
#define COMMANDER_H

#include "i2cd.h"
#include "pit.h"
#include "usartd.h"
#include "parser.h"

#define I2C_PERIOD_US 50000000
#define I2C_BUS_DEVICE_MAX_COUNT 3
#define I2C_BUFFER_MAX_SIZE 3

#define PCF_ADDRESS 0x39

typedef struct {
  unsigned int id;
  unsigned char dir;
  unsigned char writeBuffer[I2C_BUFFER_MAX_SIZE];
  unsigned char writeBufferSize;
  unsigned char readBuffer[I2C_BUFFER_MAX_SIZE];
  unsigned char readBufferSize;
} I2CNod;

class Commander
{
  private:
    static CommandVault *comvault;
    static unsigned long timestamp;
    static I2CDriver i2c;
    static I2CNod nods[I2C_BUS_DEVICE_MAX_COUNT];
    static unsigned int currentNodIdx;
    static unsigned int totalNods;
    //
    static USARTDriver comport;
    Parser parser;
    
  private:
    static void busTicker(void);
    void configurePit(void);  
    void createNod(unsigned int nodId,
                   unsigned char nodWrtiteBufferSize,
                   unsigned char nodReadBufferSize);

  public:
    Commander(CommandVault *cv);
    ~Commander();
    void start(void);
    void stop(void);
};

#endif //#ifndef COMMANDER_H
