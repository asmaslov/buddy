#ifndef PROTOCOL_H
#define PROTOCOL_H

// Unit address range [0x00 .. 0x7F]
//  Pad address range [0x80 .. 0xFF]

#define BAUDRATE 57600

#define CONTROL_PACKET_LEN 12

#define DEFAULT_UNIT_ADDR 0x0F

#define CONTROL_PACKET_PART_START     0
#define CONTROL_PACKET_PART_TYPE      1
#define CONTROL_PACKET_PART_IDX_H     2
#define CONTROL_PACKET_PART_IDX_L     3
#define CONTROL_PACKET_PART_CONTROLS  4
#define CONTROL_PACKET_PART_SPECIAL   9
#define CONTROL_PACKET_PART_CRC_H    10
#define CONTROL_PACKET_PART_CRC_L    11

#define CONTROL_PACKET_TYPE_MANUAL      0xFF
#define CONTROL_PACKET_TYPE_INSTRUCTION 0xFE

#define MAX_PACKET_INDEX 0xFFFF

#define SEGMENT_MAIN 0x00
#define SEGMENT_AUTO 0x0F

#define INSTRUCTION_MAX_LEN 20
#define INSTRUCTION_STOP_INIT 0x00
#define INSTRUCTION_CALIBRATE 0x01 
#define INSTRUCTION_GOTO      0x02
#define INSTRUCTION_REQUEST   0xFF // Warning! Autoremovable instruction

#define MAX_SIMULTANEOUS_INSTRUCTIONS 10

#define REPLY_PACKET_LEN 8

#define DEFAULT_PAD_ADDR  0x8F

#define REPLY_PACKET_PART_START   0
#define REPLY_PACKET_PART_TYPE    1
#define REPLY_PACKET_PART_IDX_H   2
#define REPLY_PACKET_PART_IDX_L   3
#define REPLY_PACKET_PART_STATUS  4
#define REPLY_PACKET_PART_SPECIAL 5
#define REPLY_PACKET_PART_CRC_H   6
#define REPLY_PACKET_PART_CRC_L   7

#define REPLY_PACKET_TYPE_AUTOREPLY 0xEF
#define REPLY_PACKET_TYPE_STATUS 0xEE
#define REPLY_PACKET_TYPE_MESSAGE 0xED

#define UNIT_STATUS_OK (1 << 0)

#define INSTRUCTION_STATUS_UNDEF     0
#define INSTRUCTION_STATUS_ACCEPTED (1 << 0)
#define INSTRUCTION_STATUS_WORKING  (1 << 1)
#define INSTRUCTION_STATUS_DONE     (1 << 2)
#define INSTRUCTION_STATUS_BREAK    (1 << 4)
#define INSTRUCTION_STATUS_ERROR    (1 << 5)

#define MESSAGE_MAX_LEN   20
#define MESSAGE_POSITION 0
#define MESSAGE_SENSORS  1 


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

typedef union {
  unsigned char bytes[CONTROL_PACKET_LEN];
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
      unsigned char codes;
      struct {
        unsigned char segment : 4;
        unsigned char crossUp : 1;
        unsigned char crossDown : 1;
        unsigned char crossLeft : 1;
        unsigned char crossRight : 1;
      };
    };
    union {
      unsigned char special;
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
    };
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
  unsigned char bytes[REPLY_PACKET_LEN];
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
      unsigned char status;
      struct {
        unsigned char ok : 1;
        unsigned char busy : 1;
        // TODO:
        // Fill status packet bits
      };
    };
    unsigned char special;
    union {
      unsigned short crc;
      struct {
        unsigned char crcL;
        unsigned char crcH;
      };
    };
  };  
} ReplyPacket;

#endif //#ifndef PROTOCOL_H
