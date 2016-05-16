#include "commander.h"
#include "pit.h"
#include "aic.h"

#include "i2cd.h"
#include "usartd.h"
#include "parser.h"

#include "delay.h"
#include "assert.h"

Commander *commanderLocal;
CommandVault *commandVaultCommander;
Comport *comportCommander;

static const Pin NodPower_pin = {BIT6, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT};

Parser parser;
I2c i2c;

static void commander_nextnod(void)
{
  SANITY_CHECK(commanderLocal);  
  commanderLocal->currentNodIdx++;
  if(commanderLocal->currentNodIdx == commanderLocal->totalNods)
  {
    commanderLocal->currentNodIdx = 0;
  } 
}

static void commander_ticker(void)
{
  SANITY_CHECK(commanderLocal);
  SANITY_CHECK(commandVaultCommander);
  unsigned int status;
  status = PIT_GetStatus() & AT91C_PITC_PITS;
  if (status != 0)
  {
    commanderLocal->timestamp += (PIT_GetPIVR() >> 20);
    if(!commanderLocal->nods[commanderLocal->currentNodIdx].disconnected)
    {
      if(commanderLocal->nods[commanderLocal->currentNodIdx].dir == TRANSFER_READ)
      {
        i2c_setAddress(commanderLocal->nods[commanderLocal->currentNodIdx].id);
        if(i2c_readNow(commanderLocal->nods[commanderLocal->currentNodIdx].readBuffer,
                       commanderLocal->nods[commanderLocal->currentNodIdx].readBufferSize))
        {
          if(!commanderLocal->nods[commanderLocal->currentNodIdx].connected)
          {
            commanderLocal->nods[commanderLocal->currentNodIdx].success++;
            if(commanderLocal->nods[commanderLocal->currentNodIdx].success == I2C_TRUST_EXCHANGE_PERIODS)
            {
              commanderLocal->nods[commanderLocal->currentNodIdx].connected = TRUE;
              commanderLocal->nods[commanderLocal->currentNodIdx].success = 0;
              TRACE_DEBUG("Nod id %d online \n\r", commanderLocal->nods[commanderLocal->currentNodIdx].id);
            }
          }
        }
        else
        {
          commanderLocal->nods[commanderLocal->currentNodIdx].connected = FALSE;
          commanderLocal->nods[commanderLocal->currentNodIdx].disconnected = TRUE;
          commanderLocal->nods[commanderLocal->currentNodIdx].attepmt = 0;
          TRACE_DEBUG("Nod id %d not connected on trying to read\n\r", commanderLocal->nods[commanderLocal->currentNodIdx].id);
        }
        if(commanderLocal->nods[commanderLocal->currentNodIdx].connected)
        {
          if(!commandVault_locked())
          {
            commandVault_lock();
            // TODO :
            // Copy values from buffer to vault according to protocol
            switch(commanderLocal->nods[commanderLocal->currentNodIdx].id)
            {
              case ENDIR12_ADDRESS:
                commandVaultCommander->status.stat12 = commanderLocal->nods[commanderLocal->currentNodIdx].readBuffer[0];
              break;
              case ENDIR34_ADDRESS:
                commandVaultCommander->status.stat34 = commanderLocal->nods[commanderLocal->currentNodIdx].readBuffer[0];
              break;
            }  
            commandVaultCommander->requests.endir12 |= (commandVaultCommander->status.stat12 & 0xC0);
            commandVaultCommander->requests.endir12 &= (commandVaultCommander->status.stat12 | 0x3F);
            commandVaultCommander->requests.endir34 |= (commandVaultCommander->status.stat34 & 0xC0);
            commandVaultCommander->requests.endir34 &= (commandVaultCommander->status.stat34 | 0x3F);
            commandVault_unlock();
          }
        }
        commanderLocal->nods[commanderLocal->currentNodIdx].dir = TRANSFER_WRITE;
      }
      else
      {
        if(commanderLocal->nods[commanderLocal->currentNodIdx].connected)
        {
          i2c_setAddress(commanderLocal->nods[commanderLocal->currentNodIdx].id);
          if(!commandVault_locked())
          {
            commandVault_lock();
            // TODO :
            // Copy command from vault to buffer according to protocol
            switch(commanderLocal->nods[commanderLocal->currentNodIdx].id)
            {
              case ENDIR12_ADDRESS:
                commanderLocal->nods[commanderLocal->currentNodIdx].writeBuffer[0] = commandVaultCommander->requests.endir12;
              break;
              case ENDIR34_ADDRESS:
                commanderLocal->nods[commanderLocal->currentNodIdx].writeBuffer[0] = commandVaultCommander->requests.endir34;
              break;
            }        
            commandVault_unlock();
          }
          if(!i2c_writeNow(commanderLocal->nods[commanderLocal->currentNodIdx].writeBuffer,
                           commanderLocal->nods[commanderLocal->currentNodIdx].writeBufferSize))
          {
            commanderLocal->nods[commanderLocal->currentNodIdx].connected = FALSE;
            commanderLocal->nods[commanderLocal->currentNodIdx].disconnected = TRUE;
            commanderLocal->nods[commanderLocal->currentNodIdx].attepmt = 0;
            TRACE_DEBUG("Nod id %d not connected on trying to write\n\r", commanderLocal->nods[commanderLocal->currentNodIdx].id);
          }
        }
        commanderLocal->nods[commanderLocal->currentNodIdx].dir = TRANSFER_READ;
        commander_nextnod();
      }
    }
    else
    {
      commanderLocal->nods[commanderLocal->currentNodIdx].attepmt++;
      if(commanderLocal->nods[commanderLocal->currentNodIdx].attepmt == I2C_RETRY_TIMEOUT_PERIODS)
      {
        commanderLocal->nods[commanderLocal->currentNodIdx].attepmt = 0;
        PIO_Clear(&NodPower_pin);
        TRACE_DEBUG("Trying to reconnect nod id %d\n\r", commanderLocal->nods[commanderLocal->currentNodIdx].id);
        commander_pause();
        i2c_disable();
        delayMs(200);
        PIO_Set(&NodPower_pin);
        i2c_enable(&i2c);
        i2c_configureMaster(I2C_FREQ_HZ);
        commander_resume();
        commanderLocal->nods[commanderLocal->currentNodIdx].disconnected = FALSE;
        commanderLocal->nods[commanderLocal->currentNodIdx].success = 0;
      }
      commander_nextnod();
    }
  }
}

void commander_init(Commander *c, CommandVault *cv, Comport *cp)
{
  SANITY_CHECK(c);
  commanderLocal = c;
  SANITY_CHECK(cv);
  commanderLocal->commandVault = cv;
  commandVaultCommander = cv;
  SANITY_CHECK(cp);
  comportCommander = cp;
  comport_setParserFunc(parser_work);
  i2c_enable(&i2c);
  i2c_configureMaster(I2C_FREQ_HZ);
  commanderLocal->timestamp = 0;
  commanderLocal->currentNodIdx = 0;
  commanderLocal->totalNods = 0;
  // TODO:
  // Create real nods
  commander_createNod(ENDIR12_ADDRESS, 1, 1);
  commander_createNod(ENDIR34_ADDRESS, 1, 1);
  parser_enable(&parser, commandVaultCommander);
}

void commander_createNod(unsigned int nodId,
                         unsigned char nodWrtiteBufferSize,
                         unsigned char nodReadBufferSize)
{
  SANITY_CHECK(commanderLocal);
  ASSERT(nodId, "-F- Nod id can not be zero\n\r");
  ASSERT(nodWrtiteBufferSize, "-F- Nod write buffer size can not be zero\n\r");
  ASSERT(nodReadBufferSize, "-F- Nod read buffer size can not be zero\n\r");
  if(commanderLocal->totalNods == I2C_BUS_DEVICE_MAX_COUNT)
  {
    TRACE_ERROR("Too many nods\n\r");
  }
  else
  {
    commanderLocal->nods[commanderLocal->totalNods].connected = FALSE;
    commanderLocal->nods[commanderLocal->totalNods].disconnected = FALSE;
    commanderLocal->nods[commanderLocal->totalNods].id = nodId;
    commanderLocal->nods[commanderLocal->totalNods].dir = TRANSFER_READ;
    commanderLocal->nods[commanderLocal->totalNods].writeBufferSize = nodWrtiteBufferSize;
    commanderLocal->nods[commanderLocal->totalNods].readBufferSize = nodReadBufferSize;
    commanderLocal->nods[commanderLocal->totalNods].success = 0;
    commanderLocal->nods[commanderLocal->totalNods].attepmt = 0;
    commanderLocal->totalNods++;
  }
}

void commander_configurePit(void)
{
  PIT_Init(I2C_PERIOD_US, BOARD_MCK / 1000000);
  AIC_DisableIT(AT91C_ID_SYS);
  AIC_ConfigureIT(AT91C_ID_SYS, AT91C_AIC_PRIOR_HIGHEST, commander_ticker);
  AIC_EnableIT(AT91C_ID_SYS);
  PIT_EnableIT();
  PIT_Enable();
}

void commander_start(void)
{
  SANITY_CHECK(commanderLocal);
  if(commanderLocal->totalNods > 0)
  {
    commander_configurePit();
  }
  else
  {
    TRACE_ERROR("Nods not created\n\r");
  }
}

void commander_stop(void)
{
  SANITY_CHECK(commanderLocal);
  PIT_DisableIT();
  commanderLocal->timestamp = 0;
  commanderLocal->currentNodIdx = 0;
}

void commander_pause(void)
{
  PIT_DisableIT();
}

void commander_resume(void)
{
  PIT_EnableIT();
}
