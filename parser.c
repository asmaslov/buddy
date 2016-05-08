#include "parser.h"
#include "comvault.h"
#include "bits.h"
#include "assert.h"

Parser *parserLocal;
CommandVault *commandVaultParser;

void parser_enable(Parser *p, CommandVault *cv)
{
  SANITY_CHECK(p);
  parserLocal = p;
  SANITY_CHECK(cv);
  commandVaultParser = cv;
  parserLocal->nextPartIdx = 0;
  parserLocal->packetRcvd = FALSE;
  parserLocal->packetGood = FALSE;
  parserLocal->needFeedback = FALSE;
}

void parser_work(unsigned char *buf, int size)
{
  SANITY_CHECK(parserLocal);
  SANITY_CHECK(commandVaultParser);  
  unsigned char recByte;
  int i;
  for(i = 0; i < size; i++)
  {
    recByte = *(buf + i);
    if(parserLocal->nextPartIdx == 9)
    {
      parserLocal->packet.crcL = recByte;
      parserLocal->nextPartIdx = 0;
      parserLocal->packetRcvd = TRUE;
    }
    if((parserLocal->nextPartIdx < 9) && (parserLocal->nextPartIdx > 1))
    {
      parserLocal->packet.bytes[parserLocal->nextPartIdx] = recByte;
      parserLocal->nextPartIdx += 1;
    }  
    if(parserLocal->nextPartIdx == 1)
    {
      if ((CONTROL_PACKET_0 == recByte)
       || (CONTROL_PACKET_1 == recByte)
       || (CONTROL_PACKET_2 == recByte)
       || (CONTROL_PACKET_3 == recByte))
      {
        parserLocal->packet.type = recByte;
        parserLocal->nextPartIdx = 2;
      }
      else
      {
        parserLocal->nextPartIdx = 0;
      }
    }
    if((parserLocal->nextPartIdx == 0) && (DEFAULT_UNIT_ADDR == recByte) && (!parserLocal->packetRcvd))
    {
      parserLocal->packet.unit = recByte;
      parserLocal->nextPartIdx = 1;
    }
  }
  if(parserLocal->packetRcvd)
  {
    parserLocal->packetRcvd = FALSE;
    // TODO:
    // Check CRC
    // unsigned char checkCRC = 0;
    parserLocal->packetGood = TRUE;
  }
  if(parserLocal->packetGood)
  {
    parserLocal->packetGood = FALSE;
    parserLocal->needFeedback = TRUE;
    switch (parserLocal->packet.type)
    {
      case CONTROL_PACKET_0:
        commandVault_lock();
        commandVaultParser->values.leftJoyX = parserLocal->packet.leftJoyXs.val * (parserLocal->packet.leftJoyXs.sign == 0 ? 1 : -1);
        commandVaultParser->values.leftJoyY = parserLocal->packet.leftJoyYs.val * (parserLocal->packet.leftJoyYs.sign == 0 ? 1 : -1);
        commandVaultParser->values.rightJoyX = parserLocal->packet.rightJoyXs.val * (parserLocal->packet.rightJoyXs.sign == 0 ? 1 : -1);
        commandVaultParser->values.rightJoyY = parserLocal->packet.rightJoyYs.val * (parserLocal->packet.rightJoyYs.sign == 0 ? 1 : -1);
        commandVaultParser->holdkeys.crossUp = parserLocal->packet.codes.crossUp;
        commandVaultParser->holdkeys.crossDown = parserLocal->packet.codes.crossDown;
        commandVaultParser->holdkeys.crossLeft = parserLocal->packet.codes.crossLeft;
        commandVaultParser->holdkeys.crossRight = parserLocal->packet.codes.crossRight;
        commandVaultParser->requests.buttonA = parserLocal->packet.requests.buttonA;
        commandVaultParser->requests.buttonB = parserLocal->packet.requests.buttonB;
        commandVaultParser->requests.buttonX = parserLocal->packet.requests.buttonX;
        commandVaultParser->requests.buttonY = parserLocal->packet.requests.buttonY;
        commandVault_unlock();
      break;      
      case CONTROL_PACKET_1:

      break;      
      case CONTROL_PACKET_2:

      break;      
      case CONTROL_PACKET_3:

      break;      
    }
  }  
}
