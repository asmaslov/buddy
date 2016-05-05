#ifndef PARSER_H
#define PARSER_H

#include "comvault.h"
#include "protocol.h"

class Parser
{ 
  private:
    static CommandVault *comvault;
    static ControlPacket packet;
    static int nextPartIdx;
    static bool packetRcvd;
    static bool packetGood;
    static bool needFeedback;
  
  public:
    Parser();
    ~Parser();
    void connectVault(CommandVault *cv);
    static void work(unsigned char *buf, int size);
};

#endif //#ifndef PARSER_H
