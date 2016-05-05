#include "commander.h"
#include "aic.h"

#include "assert.h"

CommandVault *Commander::comvault;
unsigned long Commander::timestamp;
I2CDriver Commander::i2c;
I2CNod Commander::nods[I2C_BUS_DEVICE_MAX_COUNT];
unsigned int Commander::currentNodIdx;
unsigned int Commander::totalNods;
USARTDriver Commander::comport;

Commander::Commander(CommandVault *cv)
{
  SANITY_CHECK(cv);
  comvault = cv;
  comport.configure(USART0, 57600);
  comport.setParserFunc(parser.work);
  i2c.configureMaster(I2C_FREQ_HZ);
  timestamp = 0;
  currentNodIdx = 0;
  totalNods = 0;
  // TODO:
  // Create real nods
  createNod(PCF_ADDRESS, 1, 1);
  parser.connectVault(comvault);
}

Commander::~Commander()
{
  stop();
}

void Commander::createNod(unsigned int nodId,
                          unsigned char nodWrtiteBufferSize,
                          unsigned char nodReadBufferSize)
{
  ASSERT(nodId, "-F- Nod id can not be zero\n\r");
  ASSERT(nodWrtiteBufferSize, "-F- Nod write buffer size can not be zero\n\r");
  ASSERT(nodReadBufferSize, "-F- Nod read buffer size can not be zero\n\r");
  if(totalNods == I2C_BUS_DEVICE_MAX_COUNT)
  {
    TRACE_ERROR("Too many nods\n\r");
  }
  else
  {
    nods[totalNods].id = nodId;
    nods[totalNods].dir = TRANSFER_READ;
    nods[totalNods].writeBufferSize = nodWrtiteBufferSize;
    nods[totalNods].readBufferSize = nodReadBufferSize;
    totalNods++;
  }
}

void Commander::start(void)
{
  if(totalNods > 0)
  {
    configurePit();
  }
  else
  {
    TRACE_ERROR("Nods not created\n\r");
  }
}

void Commander::stop(void)
{
  PIT_DisableIT();
  timestamp = 0;
  currentNodIdx = 0;
}

void Commander::busTicker(void)
{
  unsigned int status;
  status = PIT_GetStatus() & AT91C_PITC_PITS;
  if (status != 0)
  {
    timestamp += (PIT_GetPIVR() >> 20);
    if(nods[currentNodIdx].dir == TRANSFER_READ)
    {
      i2c.setAddress(nods[currentNodIdx].id);
      i2c.read(nods[currentNodIdx].readBuffer, nods[currentNodIdx].readBufferSize);
      // TODO :
      // Copy values from buffer to vault according to protocol
      // If vault is busy on writing -> don't copy
      if(!comvault->locked())
      {
        comvault->status.teststat = nods[currentNodIdx].readBuffer[0];
      }
      // -----
      nods[currentNodIdx].dir = TRANSFER_WRITE;
    }
    else
    {
      i2c.setAddress(nods[currentNodIdx].id);
      // TODO :
      // Copy command from vault to buffer according to protocol
      // If vault is busy on reading -> don't copy, use old values
      if(!comvault->locked())
      {
        nods[currentNodIdx].writeBuffer[0] = comvault->requests.testreq;
      }
      // -----
      i2c.write(nods[currentNodIdx].writeBuffer, nods[currentNodIdx].writeBufferSize);
      currentNodIdx++;
      if(currentNodIdx == totalNods)
      {
        currentNodIdx = 0;
      } 
      nods[currentNodIdx].dir = TRANSFER_READ;
    }
  }
}

void Commander::configurePit(void)
{
  PIT_Init(I2C_PERIOD_US, BOARD_MCK / 1000000);
  AIC_DisableIT(AT91C_ID_SYS);
  AIC_ConfigureIT(AT91C_ID_SYS, AT91C_AIC_PRIOR_HIGHEST, Commander::busTicker);
  AIC_EnableIT(AT91C_ID_SYS);
  PIT_EnableIT();
  PIT_Enable();
}
