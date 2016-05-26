#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PACKET_LEN 12

// Unit address range [0x00 .. 0x7F]
//  Pad address range [0x80 .. 0xFF]

#define DEFAULT_UNIT_ADDR 0x0F
#define DEFAULT_PAD_ADDR  0x8F

#define CONTROL_PACKET_MANUAL      0xFF
#define CONTROL_PACKET_INSTRUCTION 0xFE

#define MAX_PACKET_INDEX 0xFFFF

#define REPLY_PACKET_0 0xEF
#define REPLY_PACKET_1 0xEE
#define REPLY_PACKET_2 0xED
#define REPLY_PACKET_3 0xEC

#define SEGMENT_MAIN 0x00
#define SEGMENT_AUTO 0x0F

#define INSTRUCTION_MAX_LEN   20
#define INSTRUCTION_STOP       0
#define INSTRUCTION_CALIBRATE  1 
#define INSTRUCTION_GOTO       2

#define TOTAL_JOINTS 4

#define JOINT_X      0
#define JOINT_Y      1
#define JOINT_ZL     2
#define JOINT_ZR     3

#define JOINT_XY     0x80
#define JOINT_XYZL   0x81
#define JOINT_XYZR   0x82
#define JOINT_XYZLZR 0x83

// TODO:
// Measure it
/*#define JOINT_X_MAX
#define JOINT_Y_MAX
#define JOINT_ZR_MAX
#define JOINT_ZL_MAX*/

#define PACKET_PART_START     0
#define PACKET_PART_TYPE      1
#define PACKET_PART_IDX_H     2
#define PACKET_PART_IDX_L     3
#define PACKET_PART_CONTROLS  4
#define PACKET_PART_SPECIAL   9
#define PACKET_PART_CRC_H    10
#define PACKET_PART_CRC_L    11

typedef union {
  unsigned char bytes[PACKET_LEN];
  struct {
    unsigned char unit;
    unsigned char type;
    union {
      unsigned short idx; 
      struct {
        unsigned char idxL;
        unsigned char idxH;
      };
    };
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
      unsigned short crc;
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
    union {
      unsigned int word;
      struct {
        unsigned char l;
        unsigned char h;
      };
    } idx;
    unsigned char data[6];
    union {
      unsigned int word;
      struct {
        unsigned char l;
        unsigned char h;
      };
    } crc;
  };  
} ReplyPacket;

#endif //#ifndef PROTOCOL_H
