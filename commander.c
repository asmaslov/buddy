#include "commander.h"

#include "i2cd.h"
#include "usartd.h"
#include "parser.h"

#include "delay.h"
#include "assert.h"

static Commander *commander;
static CommandVault *commandVault;
static Comport *comport;
static ReplyPacket reply;

static const Pin NodPower_pin = {BIT6, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT};

Parser parser;
I2c i2c;

static void commander_nextnod(void)
{
  SANITY_CHECK(commander);  
  commander->currentNodIdx++;
  if(commander->currentNodIdx == commander->totalNods)
  {
    commander->currentNodIdx = 0;
  }
}

static void commander_ticker(void)
{
  SANITY_CHECK(commander);
  SANITY_CHECK(commandVault);
  if(!commander->nods[commander->currentNodIdx].disconnected)
  {
    if(commander->nods[commander->currentNodIdx].dir == TRANSFER_READ)
    {
      i2c_setAddress(commander->nods[commander->currentNodIdx].id);
    #ifdef USE_ASYNC_I2C_DRIVER
      if(i2c_read(commander->nods[commander->currentNodIdx].readBuffer,
                  commander->nods[commander->currentNodIdx].readBufferSize, NULL))
    #else
      if(i2c_readNow(commander->nods[commander->currentNodIdx].readBuffer,
                     commander->nods[commander->currentNodIdx].readBufferSize))
    #endif
      {
        if(!commander->nods[commander->currentNodIdx].connected)
        {
          commander->nods[commander->currentNodIdx].success++;
          if(commander->nods[commander->currentNodIdx].success == I2C_TRUST_EXCHANGE_PERIODS)
          {
            commander->nods[commander->currentNodIdx].connected = TRUE;
            commander->nods[commander->currentNodIdx].success = 0;
            TRACE_DEBUG("Nod id %d online\n\r", commander->nods[commander->currentNodIdx].id);
          }
        }
      }
      else
      {
        commander->nods[commander->currentNodIdx].connected = FALSE;
        commander->nods[commander->currentNodIdx].disconnected = TRUE;
        commander->nods[commander->currentNodIdx].attepmt = 0;
        TRACE_DEBUG("Nod id %d not connected on trying to read\n\r", commander->nods[commander->currentNodIdx].id);
      }
      if(commander->nods[commander->currentNodIdx].connected)
      {
        if(!commandVault_locked())
        {
          commandVault_lock();
          // TODO :
          // Copy values from buffer to vault according to protocol
          switch(commander->nods[commander->currentNodIdx].id)
          {
            case ENDIR12_ADDRESS:
              commandVault->status.stat12 = commander->nods[commander->currentNodIdx].readBuffer[0];
            break;
            case ENDIR34_ADDRESS:
              commandVault->status.stat34 = commander->nods[commander->currentNodIdx].readBuffer[0];
            break;
          }  
          commandVault->requests.endir12 |= (commandVault->status.stat12 & 0xC0);
          commandVault->requests.endir12 &= (commandVault->status.stat12 | 0x3F);
          commandVault->requests.endir34 |= (commandVault->status.stat34 & 0xC0);
          commandVault->requests.endir34 &= (commandVault->status.stat34 | 0x3F);
          comport_uputchar(commandVault->status.stat12);
          comport_uputchar(commandVault->status.stat34);
          commandVault_unlock();
        }
      }
      commander->nods[commander->currentNodIdx].dir = TRANSFER_WRITE;
    }
    else
    {
      if(commander->nods[commander->currentNodIdx].connected)
      {
        i2c_setAddress(commander->nods[commander->currentNodIdx].id);
        if(!commandVault_locked())
        {
          commandVault_lock();
          // TODO :
          // Copy command from vault to buffer according to protocol
          switch(commander->nods[commander->currentNodIdx].id)
          {
            case ENDIR12_ADDRESS:
              commander->nods[commander->currentNodIdx].writeBuffer[0] = commandVault->requests.endir12;
            break;
            case ENDIR34_ADDRESS:
              commander->nods[commander->currentNodIdx].writeBuffer[0] = commandVault->requests.endir34;
            break;
          }        
          commandVault_unlock();
        }
      #ifdef USE_ASYNC_I2C_DRIVER
        if(!i2c_write(commander->nods[commander->currentNodIdx].writeBuffer,
                         commander->nods[commander->currentNodIdx].writeBufferSize, NULL))  
      #else
        if(!i2c_writeNow(commander->nods[commander->currentNodIdx].writeBuffer,
                         commander->nods[commander->currentNodIdx].writeBufferSize))  
      #endif
        {
          commander->nods[commander->currentNodIdx].connected = FALSE;
          commander->nods[commander->currentNodIdx].disconnected = TRUE;
          commander->nods[commander->currentNodIdx].attepmt = 0;
          TRACE_DEBUG("Nod id %d not connected on trying to write\n\r", commander->nods[commander->currentNodIdx].id);
        }
      }
      commander->nods[commander->currentNodIdx].dir = TRANSFER_READ;
      commander_nextnod();
    }
  }
  else
  {
    commander->nods[commander->currentNodIdx].attepmt++;
    if(commander->nods[commander->currentNodIdx].attepmt == I2C_RETRY_TIMEOUT_PERIODS)
    {
      commander->tickerEnabled = FALSE;
      for(int i = 0; i < commander->totalNods; i++)
      {
        commander->nods[i].attepmt = 0;
        commander->nods[i].disconnected = FALSE;
        commander->nods[i].success = 0;
      }
      commander_nodsPowerDown();
      TRACE_DEBUG("Trying to reconnect nod id %d\n\r", commander->nods[commander->currentNodIdx].id);
      i2c_disable();
      delayUs(100);
      i2c_enable(&i2c);
      i2c_configureMaster(I2C_FREQ_HZ);
      commander_nodsPowerUp();
      commander->tickerEnabled = TRUE;
    }
    commander_nextnod();
  }
}

CommanderTicker commander_init(Commander *c, CommandVault *cv, Comport *cp)
{
  SANITY_CHECK(c);
  commander = c;
  SANITY_CHECK(cv);
  commander->commandVault = cv;
  commandVault = cv;
  SANITY_CHECK(cp);
  comport = cp;
  CommanderTicker ct = commander_ticker;
  comport_setParserFunc(parser_work);  
  commander->tickerEnabled = FALSE;
  commander->timer.tick = 0;
  commander->timer.compare = 0;
  commander->timer.mastertick = 0;
  commander->timer.divide = 1;  
  PIO_Configure(&NodPower_pin, 1);
  i2c_enable(&i2c);
  i2c_configureMaster(I2C_FREQ_HZ);
  commander->currentNodIdx = 0;
  commander->totalNods = 0;
  // TODO:
  // Create real nods
  commander_createNod(ENDIR12_ADDRESS, 1, 1);
  commander_createNod(ENDIR34_ADDRESS, 1, 1);
  parser_enable(&parser, commandVault);
  for(int i = 0; i < PACKET_LEN; i++)
  {
    reply.bytes[i] = 0;
  }
  return ct;
}

void commander_createNod(unsigned int nodId,
                         unsigned char nodWrtiteBufferSize,
                         unsigned char nodReadBufferSize)
{
  SANITY_CHECK(commander);
  ASSERT(nodId, "-F- Nod id can not be zero\n\r");
  ASSERT(nodWrtiteBufferSize, "-F- Nod write buffer size can not be zero\n\r");
  ASSERT(nodReadBufferSize, "-F- Nod read buffer size can not be zero\n\r");
  if(commander->totalNods == I2C_BUS_DEVICE_MAX_COUNT)
  {
    TRACE_ERROR("Too many nods\n\r");
  }
  else
  {
    commander->nods[commander->totalNods].connected = FALSE;
    commander->nods[commander->totalNods].disconnected = FALSE;
    commander->nods[commander->totalNods].id = nodId;
    commander->nods[commander->totalNods].dir = TRANSFER_READ;
    commander->nods[commander->totalNods].writeBufferSize = nodWrtiteBufferSize;
    commander->nods[commander->totalNods].readBufferSize = nodReadBufferSize;
    commander->nods[commander->totalNods].success = 0;
    commander->nods[commander->totalNods].attepmt = 0;
    commander->totalNods++;
  }
}

void commander_nodsPowerUp(void)
{
  SANITY_CHECK(commander);
  if(commander->totalNods > 0)
  {
    PIO_Clear(&NodPower_pin);
    delayMs(200);
  }
  else
  {
    TRACE_ERROR("Nods not created\n\r");
  }
}

void commander_nodsPowerDown(void)
{
  SANITY_CHECK(commander);
  PIO_Set(&NodPower_pin);
  delayMs(200);
  commander->currentNodIdx = 0;
}

void commander_reply(void)
{
  for(int i = 0; i < PACKET_LEN; i++)
  {
    comport_uputchar(reply.bytes[i]);
  }  
}
