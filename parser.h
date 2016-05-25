#ifndef PARSER_H
#define PARSER_H

#include "comvault.h"
#include "protocol.h"

typedef struct _Parser {
  ControlPacket packet;
  unsigned char nextPartIdx;
  unsigned short lastPacketIdx;
  bit packetRcvd;
  bit packetGood;
  SoftwareTimer timer;
} Parser;

void parser_enable(Parser *p, CommandVault *cv);

void parser_work(unsigned char *buf, int size);

#endif //#ifndef PARSER_H
