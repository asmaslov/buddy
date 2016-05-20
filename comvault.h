#ifndef COMVAULT_H
#define COMVAULT_H

#include "protocol.h"
#include "bits.h"

// TODO:
// Rename further structures contents according to real functionality

typedef struct _Values {
  int speedX;
  int speedY;
  int speedZR;
  int speedZL;
} Values;

typedef struct _Holdkeys {
  bit crossUp;
  bit crossDown;
  bit crossLeft;
  bit crossRight;
  bit buttonA;
  bit buttonB;
  bit buttonX;
  bit buttonY;
} Holdkeys;

typedef struct _Requests {
  bit newIns;
  bit stopAll;
  unsigned char instruction;
  unsigned char parameters[INSTRUCTION_MAX_LEN - 1];
} Requests;

typedef struct _Outputs {
  unsigned char endir12;
  unsigned char endir34;
} Outputs;

typedef struct _Status {
  bit instructionDone;
  unsigned char stat12;
  unsigned char stat34; 
} Status;

typedef struct _CommandVault {
  int key;
  unsigned int leftFeedbacks;
  Values values;
  Holdkeys holdkeys;
  Requests requests;
  Outputs outputs;
  Status status;
} CommandVault;

void commandVault_init(CommandVault *cv);

void commandVault_lock(void);

void commandVault_unlock(void);

int commandVault_locked(void);

void commander_reply(void);

#endif //#ifndef COMVAULT_H
