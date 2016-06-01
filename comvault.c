#include "comvault.h"
#include "bits.h"

#include "assert.h"

static CommandVault *commandVault;

void commandVault_init(CommandVault *cv)
{
  SANITY_CHECK(cv);
  commandVault = cv;
  commandVault->key = 0;
    
  commandVault->requests.newIns = FALSE;
  commandVault->requests.stopAll = FALSE;
  commandVault->requests.instruction = INSTRUCTION_STOP_INIT;
  
  commandVault->values.speedX = 0;
  commandVault->values.speedY = 0;
  commandVault->values.speedZL = 0;
  commandVault->values.speedZR = 0;
  
  commandVault->holdkeys.crossUp = 0;
  commandVault->holdkeys.crossDown = 0;
  commandVault->holdkeys.crossLeft = 0;
  commandVault->holdkeys.crossRight = 0;
  commandVault->holdkeys.buttonA = 0;
  commandVault->holdkeys.buttonB = 0;
  commandVault->holdkeys.buttonX = 0;
  commandVault->holdkeys.buttonY = 0;

  commandVault->outputs.endir12 = 0;
  commandVault->outputs.endir34 = 0;
  
  commandVault->status.ready = FALSE;
  commandVault->status.instructionDone = FALSE;
}

void commandVault_lock(void)
{
  SANITY_CHECK(commandVault);
  while(commandVault->key);
  commandVault->key = 1;
}

void commandVault_unlock(void)
{
  SANITY_CHECK(commandVault);
  commandVault->key = 0;
}

int commandVault_locked(void)
{
  SANITY_CHECK(commandVault);
  return commandVault->key;
}
