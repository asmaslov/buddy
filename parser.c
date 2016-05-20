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
  parser->nextPartIdx = 0;
  parser->packetRcvd = FALSE;
  parser->packetGood = FALSE;
  commandVault->leftFeedbacks = 0;
}

void parser_work(unsigned char *buf, int size)
{
  SANITY_CHECK(parser);
  SANITY_CHECK(commandVault);  
  unsigned char recByte;
  int i;
  for(i = 0; i < size; i++)
  {
    recByte = *(buf + i);
    if(parser->nextPartIdx == 9)
    {
      parser->packet.crcL = recByte;
      parser->nextPartIdx = 0;
      parser->packetRcvd = TRUE;
    }
    if(parser->nextPartIdx == 8)
    {
      parser->packet.crcH = recByte;
      parser->nextPartIdx = 9;
    }
    if(parser->nextPartIdx == 7)
    {
      switch (parser->packet.type)
      {
        case CONTROL_PACKET_MANUAL:
          parser->packet.special.byte = recByte;
          parser->nextPartIdx = 8;    
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
              parser->nextPartIdx = 8;    
            }
          }
          else
          {
            instruction[instructionIdx++] = recByte;
            if(instructionIdx == instructionLen)
            {
              parser->nextPartIdx = 8;
            }
          }
        break;
      }
    }
    if((parser->nextPartIdx < 7) && (parser->nextPartIdx > 1))
    {
      parser->packet.bytes[parser->nextPartIdx] = recByte;
      parser->nextPartIdx += 1;
    } 
    if(parser->nextPartIdx == 1)
    {
      if ((CONTROL_PACKET_MANUAL == recByte)
       || (CONTROL_PACKET_INSTRUCTION == recByte))
      {
        parser->packet.type = recByte;
        parser->nextPartIdx = 2;
      }
      else
      {
        parser->nextPartIdx = 0;
      }
    }
    if((parser->nextPartIdx == 0) && (DEFAULT_UNIT_ADDR == recByte) && (!parser->packetRcvd))
    {
      parser->packet.unit = recByte;
      parser->nextPartIdx = 1;
    }
  }
  if(parser->packetRcvd)
  {
    parser->packetRcvd = FALSE;
    unsigned int checkCRC = 0;
    switch (parser->packet.type)
    {
      case CONTROL_PACKET_MANUAL:
        for(int i = 0; i < PACKET_LEN - 2; i++)
        {
          checkCRC += parser->packet.bytes[i];
        }

        if(checkCRC == parser->packet.crc)
        {
          parser->packetGood = TRUE;
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
          parser->packetGood = TRUE;
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
