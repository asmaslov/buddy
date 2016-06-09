#ifndef COMVAULT_H
#define COMVAULT_H

#include "protocol.h"
#include "bits.h"

// TODO:
// Rename further structures contents according to real functionality

#define INSTRUCTIONS_MEMORY_SLOTS 16

typedef struct {
  bit enabled;
  unsigned long tick;
  unsigned long compare;
  unsigned int mastertick;
  unsigned int divide;
} SoftwareTimer;

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

typedef struct _Instruction {
  unsigned short idx;
  unsigned char code;
  unsigned char parameters[INSTRUCTION_MAX_LEN - 1];
  unsigned char condition;
  struct _Instruction *next;
} Instruction;

typedef struct _Requests {
  Instruction *instructions;
  int totalInstructions;
} Requests;

typedef struct _Outputs {
  unsigned char endir12;
  unsigned char endir34;
} Outputs;

typedef struct _Status {
  bit ok;
  bit busy;
  unsigned char messageLen;
  unsigned char message[MESSAGE_MAX_LEN];
  unsigned char stat12;
  unsigned char stat34;
} Status;

typedef struct _CommandVault {
  int key;
  unsigned int leftFeedbacks;
  unsigned short lastPacketIdx;
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

bit addInstruction(Instruction *ins);

bit removeInstructionByIdx(unsigned short idx);

Instruction *getInstruction(int pos);

Instruction *getInstructionByIdx(unsigned short idx);

#endif //#ifndef COMVAULT_H
