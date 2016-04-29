#ifndef PARSER_H
#define PARSER_H

#define PACKET_LEN 10

#define DEFAULT_ADDR 0x0F
#define PACKET_0 0xFF
#define PACKET_1 0xFE
#define PACKET_2 0xFD
#define PACKET_3 0xFC

// TODO: Name further 3 structure contents according to real functionality
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
} Requests;

class CommandVault 
{  
  private:
    int key;
  
  public:
    CommandVault();
    ~CommandVault();
    void lock(void);
    void unlock(void);
    
  public:
    Values values;
    Holdkeys holdkeys;
    Requests requests;
    
};

typedef struct {
  unsigned char unit;
  unsigned char type;
  union {
    unsigned char byte;
    struct {
      unsigned char val  : 7;
      unsigned char sign : 1;
    };
  } leftJoyXs;
  union {
    unsigned char byte;
    struct {
      unsigned char val  : 7;
      unsigned char sign : 1;
    };
  } leftJoyYs;
  union {
    unsigned char byte;
    struct {
      unsigned char val  : 7;
      unsigned char sign : 1;
    };
  } rightJoyXs;
  union {
    unsigned char byte;
    struct {
      unsigned char val  : 7;
      unsigned char sign : 1;
    };
  } rightJoyYs;
  union {
    unsigned char byte;
    struct {
      unsigned char crossUp : 1;
      unsigned char crossDown : 1;
      unsigned char crossLeft : 1;
      unsigned char crossRight : 1;
      unsigned char segment : 4;
    };
  } codes;
  union {
    unsigned char byte;
    struct {
      unsigned char buttonA : 1;
      unsigned char buttonB : 1;
      unsigned char buttonX : 1;
      unsigned char buttonY : 1;
      unsigned char buttonC : 1;
      unsigned char buttonD : 1;
      unsigned char buttonL : 1;
      unsigned char buttonR : 1;
    };
  } requests;
  unsigned char crcH;
  unsigned char crcL;
} ControlPacket;

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
    Parser(CommandVault *cv);
    ~Parser();
    static void work(unsigned char *buf, int size);
};

#endif //#ifndef PARSER_H