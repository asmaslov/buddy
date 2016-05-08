#include "commander.h"
#include "pit.h"
#include "aic.h"

#include "i2cd.h"
#include "usartd.h"
#include "parser.h"

#include "assert.h"

Commander *commanderLocal;
CommandVault *commandVaultCommander;
Comport *comportCommander;

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
    if(commanderLocal->nods[commanderLocal->currentNodIdx].connected)
    {
      if(commanderLocal->nods[commanderLocal->currentNodIdx].dir == TRANSFER_READ)
      {
        i2c_setAddress(commanderLocal->nods[commanderLocal->currentNodIdx].id);
        if(!i2c_readNow(commanderLocal->nods[commanderLocal->currentNodIdx].readBuffer,
                        commanderLocal->nods[commanderLocal->currentNodIdx].readBufferSize))
        {
          commanderLocal->nods[commanderLocal->currentNodIdx].connected = FALSE;
          TRACE_DEBUG("Nod id %d not connected\n\r", commanderLocal->nods[commanderLocal->currentNodIdx].id);
        }
        commanderLocal->nods[commanderLocal->currentNodIdx].dir = TRANSFER_WRITE;
        // TODO :
        // Copy values from buffer to vault according to protocol
        // If vault is busy on writing -> don't copy
        if(!commandVault_locked())
        {
          commandVaultCommander->status.teststat = commanderLocal->nods[commanderLocal->currentNodIdx].readBuffer[0];
        }
      }
      else
      {
        i2c_setAddress(commanderLocal->nods[commanderLocal->currentNodIdx].id);
        // TODO :
        // Copy command from vault to buffer according to protocol
        // If vault is busy on reading -> don't copy, use old values
        if(!commandVault_locked())
        {
          commanderLocal->nods[commanderLocal->currentNodIdx].writeBuffer[0] = commandVaultCommander->requests.testreq;
        }
        // -----
        if(!i2c_writeNow(commanderLocal->nods[commanderLocal->currentNodIdx].writeBuffer,
                         commanderLocal->nods[commanderLocal->currentNodIdx].writeBufferSize))
        {
          commanderLocal->nods[commanderLocal->currentNodIdx].connected = FALSE;
          TRACE_DEBUG("Nod id %d not connected\n\r", commanderLocal->nods[commanderLocal->currentNodIdx].id);
        }
        commanderLocal->nods[commanderLocal->currentNodIdx].dir = TRANSFER_READ;
        commander_nextnod();
      }
    }
    else
    {
      //TRACE_DEBUG("Nod id %d not connected\n\r", commanderLocal->nods[commanderLocal->currentNodIdx].id);
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
  comport_enable(comportCommander);
  comport_configure(USART0, 57600);
  comport_setParserFunc(parser_work);
  i2c_enable(&i2c);
  i2c_configureMaster(I2C_FREQ_HZ);
  commanderLocal->timestamp = 0;
  commanderLocal->currentNodIdx = 0;
  commanderLocal->totalNods = 0;
  // TODO:
  // Create real nods
  commander_createNod(PCF_ADDRESS, 1, 1);
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
    commanderLocal->nods[commanderLocal->totalNods].connected = TRUE;
    commanderLocal->nods[commanderLocal->totalNods].id = nodId;
    commanderLocal->nods[commanderLocal->totalNods].dir = TRANSFER_READ;
    commanderLocal->nods[commanderLocal->totalNods].writeBufferSize = nodWrtiteBufferSize;
    commanderLocal->nods[commanderLocal->totalNods].readBufferSize = nodReadBufferSize;
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
