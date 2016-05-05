#include "comvault.h"

CommandVault::CommandVault()
{
  key = 0;
  values.leftJoyX = 0;
  values.leftJoyY = 0;
  values.rightJoyX = 0;
  values.rightJoyY = 0;
  holdkeys.crossUp = 0;
  holdkeys.crossDown = 0;
  holdkeys.crossLeft = 0;
  holdkeys.crossRight = 0;
  requests.buttonA = 0;
  requests.buttonB = 0;
  requests.buttonX = 0;
  requests.buttonY = 0;

  requests.testreq = 0;
}

void CommandVault::lock()
{
  while(key);
  key = 1;
}

void CommandVault::unlock()
{
  key = 0;
}

int CommandVault::locked()
{
  return key;
}
