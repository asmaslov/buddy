#ifndef COMVAULT_H
#define COMVAULT_H

// TODO:
// Rename further 3 structure contents according to real functionality

typedef struct {
  int leftJoyX;
  int leftJoyY;
  int rightJoyX;
  int rightJoyY;
} Values;

typedef struct {
  unsigned char crossUp;
  unsigned char crossDown;
  unsigned char crossLeft;
  unsigned char crossRight;
} Holdkeys;

typedef struct {
  unsigned char buttonA;
  unsigned char buttonB;
  unsigned char buttonX;
  unsigned char buttonY;
  
  unsigned char testreq;
} Requests;

typedef struct {
  unsigned char teststat;
} Status;

class CommandVault 
{  
  private:
    int key;
  
  public:
    CommandVault();
    ~CommandVault();
    void lock(void);
    void unlock(void);
    int locked(void);
    
  public:
    Values values;
    Holdkeys holdkeys;
    Requests requests;
    Status status;
    
};

#endif //#ifndef COMVAULT_H
