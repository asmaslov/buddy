#include "comvault.h"
#include "bits.h"

#include "assert.h"

static CommandVault *commandVault;

void commandVault_init(CommandVault *cv)
{
  SANITY_CHECK(cv);
  commandVault = cv;
  commandVault->key = 0;
  commandVault->needFeedback = FALSE;
  commandVault->values.leftJoyX = 0;
  commandVault->values.leftJoyY = 0;
  commandVault->values.rightJoyX = 0;
  commandVault->values.rightJoyY = 0;
  commandVault->holdkeys.crossUp = 0;
  commandVault->holdkeys.crossDown = 0;
  commandVault->holdkeys.crossLeft = 0;
  commandVault->holdkeys.crossRight = 0;
  commandVault->requests.buttonA = 0;
  commandVault->requests.buttonB = 0;
  commandVault->requests.buttonX = 0;
  commandVault->requests.buttonY = 0;

  commandVault->requests.endir12 = 0;
  commandVault->requests.endir34 = 0;
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
