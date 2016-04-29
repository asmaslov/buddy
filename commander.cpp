#include "commander.h"
#include "usartd.h"

#include "assert.h"

CommandVault *Commander::comvault;

Commander::Commander(CommandVault *cv)
{
  SANITY_CHECK(cv);
  comvault = cv;
  i2c.configureMaster();
}

Commander::~Commander()
{

}

void Commander::start(void)
{


}
