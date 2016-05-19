#ifndef COMVAULT_H
#define COMVAULT_H

// TODO:
// Rename further 3 structure contents according to real functionality

typedef struct _Values {
  int speedX;
  int speedY;
  int speedZR;
  int speedZL;
} Values;

typedef struct _Holdkeys {
  unsigned char crossUp;
  unsigned char crossDown;
  unsigned char crossLeft;
  unsigned char crossRight;
  unsigned char buttonA;
  unsigned char buttonB;
  unsigned char buttonX;
  unsigned char buttonY;
} Holdkeys;

typedef struct _Requests {
  unsigned char new;
  unsigned char stop;
  unsigned char calibrate;
} Requests;

typedef struct _Outputs {
  unsigned char endir12;
  unsigned char endir34;
} Outputs;

typedef struct _Status {
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
