#ifndef PARSER_H
#define PARSER_H

#include "comvault.h"
#include "protocol.h"

typedef struct _Parser {
  ControlPacket packet;
  unsigned char nextPartIdx;
  SoftwareTimer timer;
} Parser;

void parser_enable(Parser *p, CommandVault *cv);

void parser_work(char *buf, int size);

#endif //#ifndef PARSER_H
