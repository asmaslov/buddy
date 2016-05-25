#include "parser.h"
#include "comvault.h"
#include "bits.h"
#include "assert.h"

static Parser *parser;
static CommandVault *commandVault;

static unsigned char instruction[INSTRUCTION_MAX_LEN];
static unsigned char instructionLen = 0;
static unsigned char instructionIdx = 0;

void parser_enable(Parser *p, CommandVault *cv)
{
  SANITY_CHECK(p);
  parser = p;
  SANITY_CHECK(cv);
  commandVault = cv;
  ASSERT((sizeof(ControlPacket) == PACKET_LEN), "Packet length not equal to real structure size\n\r");
  parser->nextPartIdx = 0;
  parser->lastPacketIdx = 0;
  parser->packetRcvd = FALSE;
  parser->packetGood = FALSE;
  parser->timer.enabled = FALSE;
  parser->timer.tick = 0;
  parser->timer.compare = 0;
  parser->timer.mastertick = 0;
  parser->timer.divide = 1; 
  commandVault->leftFeedbacks = 0;
}

void parser_work(unsigned char *buf, int size)
{
  SANITY_CHECK(parser);
  SANITY_CHECK(commandVault);  
  unsigned char recByte;
  for(int i = 0; i < size; i++)
  {
    recByte = *(buf + i);
    if(parser->nextPartIdx == PACKET_PART_CRC_L)
    {
      parser->packet.crcL = recByte;
      parser->nextPartIdx = PACKET_PART_START;
      parser->packetRcvd = TRUE;
    }
    if(parser->nextPartIdx == PACKET_PART_CRC_H)
    {
      parser->packet.crcH = recByte;
      parser->nextPartIdx++;
    }
    if(parser->nextPartIdx == PACKET_PART_SPECIAL)
    {
      switch (parser->packet.type)
      {
        case CONTROL_PACKET_MANUAL:
          parser->packet.special.byte = recByte;
          parser->nextPartIdx++;
        break;
        case CONTROL_PACKET_INSTRUCTION:
          if(instructionLen == 0)
          {
            if(recByte != 0)
            {
              instructionLen = recByte;
              instructionIdx = 0;
            }
            else
            {
              parser->packet.special.byte = 0;
              parser->nextPartIdx++;
            }
          }
          else
          {
            instruction[instructionIdx] = recByte;
            if(++instructionIdx == instructionLen)
            {
              parser->nextPartIdx++;
            }
          }
        break;
      }
    }
    if((parser->nextPartIdx < PACKET_PART_SPECIAL) && (parser->nextPartIdx >= PACKET_PART_CONTROLS))
    {
      parser->packet.bytes[parser->nextPartIdx] = recByte;
      parser->nextPartIdx++;
    } 
    if(parser->nextPartIdx == PACKET_PART_IDX_L)
    {
      parser->packet.idxL = recByte;
      parser->nextPartIdx++;
    }
    if(parser->nextPartIdx == PACKET_PART_IDX_H)
    {
      parser->packet.idxH = recByte;
      parser->nextPartIdx++;
    }
    if(parser->nextPartIdx == PACKET_PART_TYPE)
    {
      if ((CONTROL_PACKET_MANUAL == recByte)
       || (CONTROL_PACKET_INSTRUCTION == recByte))
      {
        parser->packet.type = recByte;
        parser->nextPartIdx++;
        instructionLen = 0;
      }
      else
      {
        parser->nextPartIdx = PACKET_PART_START;
      }
    }
    if((parser->nextPartIdx == PACKET_PART_START) && (DEFAULT_UNIT_ADDR == recByte) && (!parser->packetRcvd))
    {
      parser->packet.unit = recByte;
      parser->nextPartIdx++;
    }
  }
  if(parser->packetRcvd)
  {
    parser->packetRcvd = FALSE;
    unsigned short checkCRC = 0;
    switch (parser->packet.type)
    {
      case CONTROL_PACKET_MANUAL:
        for(int i = 0; i < PACKET_LEN - 2; i++)
        {
          checkCRC += parser->packet.bytes[i];
        }
        if(checkCRC == parser->packet.crc)
        {
          if((parser->packet.idx > parser->lastPacketIdx) ||
             ((parser->packet.idx == 0) && (parser->lastPacketIdx == MAX_PACKET_INDEX)))
          {    
            parser->lastPacketIdx = parser->packet.idx;
            parser->packetGood = TRUE;
          }
        }
      break;
      case CONTROL_PACKET_INSTRUCTION:
        for(int i = 0; i < PACKET_LEN - 3; i++)
        {
          checkCRC += parser->packet.bytes[i];
        }
        checkCRC += instructionLen;
        for(int i = 0; i < instructionLen; i++)
        {
          checkCRC += instruction[i];
        }
        if(checkCRC == parser->packet.crc)
        {
          if((parser->packet.idx > parser->lastPacketIdx) ||
             ((parser->packet.idx == 0) && (parser->lastPacketIdx == MAX_PACKET_INDEX)))
          {    
            parser->lastPacketIdx = parser->packet.idx;
            parser->packetGood = TRUE;
          }
        }
      break;
    }
  }
  if(parser->packetGood)
  {
    parser->packetGood = FALSE; 
    commandVault_lock();
    switch (parser->packet.codes.segment)
    {
      case SEGMENT_MAIN:
        commandVault->values.speedX = parser->packet.leftJoyX.val * (parser->packet.leftJoyX.sign == 0 ? 1 : -1);
        commandVault->values.speedY = parser->packet.leftJoyY.val * (parser->packet.leftJoyY.sign == 0 ? 1 : -1);
        commandVault->values.speedZR = parser->packet.rightJoyX.val * (parser->packet.rightJoyX.sign == 0 ? 1 : -1);
        commandVault->values.speedZL = parser->packet.rightJoyY.val * (parser->packet.rightJoyY.sign == 0 ? 1 : -1);
        commandVault->holdkeys.crossUp = parser->packet.codes.crossUp;
        commandVault->holdkeys.crossDown = parser->packet.codes.crossDown;
        commandVault->holdkeys.crossLeft = parser->packet.codes.crossLeft;
        commandVault->holdkeys.crossRight = parser->packet.codes.crossRight;
      break;
      case SEGMENT_AUTO:
      
      break;
      default:
        commandVault->values.speedX = parser->packet.leftJoyX.val * (parser->packet.leftJoyX.sign == 0 ? 1 : -1);
        commandVault->values.speedY = parser->packet.leftJoyY.val * (parser->packet.leftJoyY.sign == 0 ? 1 : -1);
        commandVault->values.speedZR = parser->packet.rightJoyX.val * (parser->packet.rightJoyX.sign == 0 ? 1 : -1);
        commandVault->values.speedZL = parser->packet.rightJoyY.val * (parser->packet.rightJoyY.sign == 0 ? 1 : -1);
        commandVault->holdkeys.crossUp = parser->packet.codes.crossUp;
        commandVault->holdkeys.crossDown = parser->packet.codes.crossDown;
        commandVault->holdkeys.crossLeft = parser->packet.codes.crossLeft;
        commandVault->holdkeys.crossRight = parser->packet.codes.crossRight;
    }    
    switch (parser->packet.type)
    {
      case CONTROL_PACKET_MANUAL:
        commandVault->holdkeys.buttonA = parser->packet.special.buttonA;
        commandVault->holdkeys.buttonB = parser->packet.special.buttonB;
        commandVault->holdkeys.buttonX = parser->packet.special.buttonX;
        commandVault->holdkeys.buttonY = parser->packet.special.buttonY;
      break;      
      case CONTROL_PACKET_INSTRUCTION:
        if(instructionLen != 0)
        {
          if(instruction[0] == INSTRUCTION_STOP)
          {
            commandVault->requests.stopAll = TRUE;
          }
          if(!commandVault->requests.newIns)
          {
            commandVault->requests.newIns = TRUE;
            TRACE_DEBUG("New instruction\n\r");
            commandVault->status.instructionDone = FALSE;
            commandVault->requests.instruction = instruction[0];
            for(int i = 0; i < instructionLen - 1; i++)
            {
              commandVault->requests.parameters[i] = instruction[i + 1];
            }
          }
          else
          {
            TRACE_DEBUG("Manipulator busy\n\r");
          }
          instructionLen = 0;          
          commandVault->leftFeedbacks++;
        }
      break;            
    }
    commandVault_unlock();
  }  
}
