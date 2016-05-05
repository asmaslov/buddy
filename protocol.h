#ifndef PROTOCOL_H
#define PROTOCOL_H

#define PACKET_LEN 10

// Unit address range [0x00 .. 0x7F]
// Pad address range  [0x80 .. 0xFF]

#define DEFAULT_UNIT_ADDR 0x0F
#define DEFAULT_PAD_ADDR 0x8F

#define CONTROL_PACKET_0 0xFF
#define CONTROL_PACKET_1 0xFE
#define CONTROL_PACKET_2 0xFD
#define CONTROL_PACKET_3 0xFC

#define REPLY_PACKET_0 0xEF
#define REPLY_PACKET_1 0xEE
#define REPLY_PACKET_2 0xED
#define REPLY_PACKET_3 0xEC

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
        unsigned char buttonL1 : 1;
        unsigned char buttonL2 : 1;
        unsigned char buttonR1 : 1;
        unsigned char buttonR2 : 1;
      };
    } requests;
    unsigned char crcH;
    unsigned char crcL;
  };  
} ControlPacket;

typedef union {
  unsigned char bytes[PACKET_LEN];
  struct {
    unsigned char unit;
    unsigned char type;
    // TODO:
    // Create 6 more bytes of data
    unsigned char crcH;
    unsigned char crcL;
  };  
} ReplyPacket;

#endif //#ifndef PROTOCOL_H
