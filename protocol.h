#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PACKET_LEN 10

// Unit address range [0x00 .. 0x7F]
//  Pad address range [0x80 .. 0xFF]

#define DEFAULT_UNIT_ADDR 0x0F
#define DEFAULT_PAD_ADDR 0x8F

#define CONTROL_PACKET_MANUAL 0xFF
#define CONTROL_PACKET_INSTRUCTION 0xFE

#define INSTRUCTION_MAX_LEN 256

#define REPLY_PACKET_0 0xEF
#define REPLY_PACKET_1 0xEE
#define REPLY_PACKET_2 0xED
#define REPLY_PACKET_3 0xEC

#define INSTRUCTION_GOTO_XY  0
#define INSTRUCTION_R_GOTO_Z 1
#define INSTRUCTION_L_GOTO_Z 2

typedef union {
  unsigned char bytes[PACKET_LEN];
  struct {
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
        unsigned char segment : 4;
        unsigned char crossUp : 1;
        unsigned char crossDown : 1;
        unsigned char crossLeft : 1;
        unsigned char crossRight : 1;
      };
    } codes;
    union {
      unsigned char byte;
      struct {
        unsigned char buttonA : 1;
        unsigned char buttonB : 1;
        unsigned char buttonX : 1;
        unsigned char buttonY : 1;
        unsigned char buttonL1 : 1;
        unsigned char buttonL2 : 1;
        unsigned char buttonR1 : 1;
        unsigned char buttonR2 : 1;
      };
    } special;
    unsigned char crcH;
    unsigned char crcL;
  };  
} ControlPacket;

typedef union {
  unsigned char bytes[PACKET_LEN];
  struct {
    unsigned char unit;
    unsigned char type;
    unsigned char data[6];
    unsigned char crcH;
    unsigned char crcL;
  };  
} ReplyPacket;

#endif //#ifndef PROTOCOL_H
