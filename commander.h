#ifndef COMMANDER_H
#define COMMANDER_H

#include "i2cd.h"
#include "parser.h"

class Commander
{
  private:
    static CommandVault *comvault;
    I2CDriver i2c;

  public:
    Commander(CommandVault *cv);
    ~Commander();
    void start(void);
};

#endif //#ifndef COMMANDER_H
