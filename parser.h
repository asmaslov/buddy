#ifndef PARSER_H
#define PARSER_H

#include "comvault.h"
#include "protocol.h"

typedef struct _Parser {
  ControlPacket packet;
  int nextPartIdx;
  int packetRcvd;
  int packetGood;
  int needFeedback;
} Parser;

void parser_enable(Parser *p, CommandVault *cv);

void parser_work(unsigned char *buf, int size);

#endif //#ifndef PARSER_H
