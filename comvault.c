#include "comvault.h"

#include "assert.h"

CommandVault *commandVaultLocal;

void commandVault_init(CommandVault *cv)
{
  SANITY_CHECK(cv);
  commandVaultLocal = cv;
  commandVaultLocal->key = 0;
  commandVaultLocal->values.leftJoyX = 0;
  commandVaultLocal->values.leftJoyY = 0;
  commandVaultLocal->values.rightJoyX = 0;
  commandVaultLocal->values.rightJoyY = 0;
  commandVaultLocal->holdkeys.crossUp = 0;
  commandVaultLocal->holdkeys.crossDown = 0;
  commandVaultLocal->holdkeys.crossLeft = 0;
  commandVaultLocal->holdkeys.crossRight = 0;
  commandVaultLocal->requests.buttonA = 0;
  commandVaultLocal->requests.buttonB = 0;
  commandVaultLocal->requests.buttonX = 0;
  commandVaultLocal->requests.buttonY = 0;

  commandVaultLocal->requests.testreq = 0;
}

void commandVault_lock(void)
{
  SANITY_CHECK(commandVaultLocal);
  while(commandVaultLocal->key);
  commandVaultLocal->key = 1;
}

void commandVault_unlock(void)
{
  SANITY_CHECK(commandVaultLocal);
  commandVaultLocal->key = 0;
}

int commandVault_locked(void)
{
  SANITY_CHECK(commandVaultLocal);
  return commandVaultLocal->key;
}
