#ifndef COMVAULT_H
#define COMVAULT_H

// TODO:
// Rename further 3 structure contents according to real functionality

typedef struct _Values {
  int leftJoyX;
  int leftJoyY;
  int rightJoyX;
  int rightJoyY;
} Values;

typedef struct _Holdkeys {
  unsigned char crossUp;
  unsigned char crossDown;
  unsigned char crossLeft;
  unsigned char crossRight;
} Holdkeys;

typedef struct _Requests {
  unsigned char buttonA;
  unsigned char buttonB;
  unsigned char buttonX;
  unsigned char buttonY;
  
  unsigned char endir12;
  unsigned char endir34;
} Requests;

typedef struct _Status {
  unsigned char stat12;
  unsigned char stat34; 
} Status;

typedef struct _CommandVault {
  int key;
  Values values;
  Holdkeys holdkeys;
  Requests requests;
  Status status;
} CommandVault;

void commandVault_init(CommandVault *cv);

void commandVault_lock(void);

void commandVault_unlock(void);

int commandVault_locked(void);

#endif //#ifndef COMVAULT_H
