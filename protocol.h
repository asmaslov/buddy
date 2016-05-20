#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PACKET_LEN 10

// Unit address range [0x00 .. 0x7F]
//  Pad address range [0x80 .. 0xFF]

#define DEFAULT_UNIT_ADDR 0x0F
#define DEFAULT_PAD_ADDR 0x8F

#define CONTROL_PACKET_MANUAL 0xFF
#define CONTROL_PACKET_INSTRUCTION 0xFE

#define REPLY_PACKET_0 0xEF
#define REPLY_PACKET_1 0xEE
#define REPLY_PACKET_2 0xED
#define REPLY_PACKET_3 0xEC

#define INSTRUCTION_MAX_LEN 256
#define INSTRUCTION_STOP       0
#define INSTRUCTION_CALIBRATE  1 
#define INSTRUCTION_GOTO       2

#define TOTAL_JOINTS 4
#define JOINT_X      0
#define JOINT_Y      1
#define JOINT_ZL     2
#define JOINT_ZR     3
#define JOINT_XY     4
#define JOINT_XYZL   5
#define JOINT_XYZR   6
#define JOINT_XYZLZR 7

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
    } leftJoyX;
    union {
      unsigned char byte;
      struct {
        unsigned char val  : 7;
        unsigned char sign : 1;
      };
    } leftJoyY;
    union {
      unsigned char byte;
      struct {
        unsigned char val  : 7;
        unsigned char sign : 1;
      };
    } rightJoyX;
    union {
      unsigned char byte;
      struct {
        unsigned char val  : 7;
        unsigned char sign : 1;
      };
    } rightJoyY;
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
    union {
      unsigned int crc;
      struct {
        unsigned char crcL;
        unsigned char crcH;
      };
    };
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
