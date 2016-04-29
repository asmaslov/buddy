#include "parser.h"
#include "assert.h"
    
CommandVault *Parser::comvault;
ControlPacket Parser::packet;
int Parser::nextPartIdx;
bool Parser::packetRcvd;
bool Parser::packetGood;
bool Parser::needFeedback;

CommandVault::CommandVault()
{
  key = 0;
  values.leftJoyX = 0;
  values.leftJoyY = 0;
  values.rightJoyX = 0;
  values.rightJoyY = 0;
  holdkeys.crossUp = 0;
  holdkeys.crossDown = 0;
  holdkeys.crossLeft = 0;
  holdkeys.crossRight = 0;
  requests.buttonA = 0;
  requests.buttonB = 0;
  requests.buttonX = 0;
  requests.buttonY = 0;
}

void CommandVault::lock()
{
  while(key);
  key = 1;
}

void CommandVault::unlock()
{
  key = 0;
}

Parser::Parser(CommandVault *cv)
{
  SANITY_CHECK(cv);
  comvault = cv;
  nextPartIdx = 0;
  packetRcvd = false;
  packetGood = false;
  needFeedback = false;  
}

Parser::~Parser()
{

}

void Parser::work(unsigned char *buf, int size)
{
  unsigned char recByte;
  int i;
  for(i = 0; i < size; i++)
  {
    recByte = *(buf + i);
    if (nextPartIdx == 9)
    {
      packet.crcL = recByte;
      nextPartIdx = 0;
      packetRcvd = true;
    }
    if (nextPartIdx == 8)
    {
      packet.crcH = recByte;
      nextPartIdx = 9;
    }
    if (nextPartIdx == 7)
    {
      packet.requests.byte = recByte;
      nextPartIdx = 8;
    }
    if (nextPartIdx == 6)
    {
      packet.codes.byte = recByte;
      nextPartIdx = 7;
    }
    if (nextPartIdx == 5)
    {
      packet.rightJoyYs.byte = recByte;
      nextPartIdx = 6;
    }
    if (nextPartIdx == 4)
    {
      packet.rightJoyXs.byte = recByte;
      nextPartIdx = 5;
    }
    if (nextPartIdx == 3)
    {
      packet.leftJoyYs.byte = recByte;
      nextPartIdx = 4;
    }
    if (nextPartIdx == 2)
    {
      packet.leftJoyXs.byte = recByte;
      nextPartIdx = 3;
    }
    if (nextPartIdx == 1)
    {
      if ((PACKET_0 == recByte)
       || (PACKET_1 == recByte)
       || (PACKET_2 == recByte)
       || (PACKET_3 == recByte))
      {
        packet.type = recByte;
        nextPartIdx = 2;
      }
      else
      {
        nextPartIdx = 0;
      }
    }
    if ((nextPartIdx == 0) && (DEFAULT_ADDR == recByte) && (!packetRcvd))
    {
      packet.unit = recByte;
      nextPartIdx = 1;
    }
  }
  if (packetRcvd)
  {
    packetRcvd = false;
    // TODO: Check CRC
    // unsigned char checkCRC = 0;
    packetGood = true;
  }
  if (packetGood)
  {
    packetGood = false;
    needFeedback = true;
    switch (packet.type)
    {
      case PACKET_0:
        comvault->lock();
        comvault->values.leftJoyX = packet.leftJoyXs.val * (packet.leftJoyXs.sign == 0 ? 1 : -1);
        comvault->values.leftJoyY = packet.leftJoyYs.val * (packet.leftJoyYs.sign == 0 ? 1 : -1);
        comvault->values.rightJoyX = packet.rightJoyXs.val * (packet.rightJoyXs.sign == 0 ? 1 : -1);
        comvault->values.rightJoyY = packet.rightJoyYs.val * (packet.rightJoyYs.sign == 0 ? 1 : -1);
        comvault->holdkeys.crossUp = packet.codes.crossUp;
        comvault->holdkeys.crossDown = packet.codes.crossDown;
        comvault->holdkeys.crossLeft = packet.codes.crossLeft;
        comvault->holdkeys.crossRight = packet.codes.crossRight;
        comvault->requests.buttonA = packet.requests.buttonA;
        comvault->requests.buttonB = packet.requests.buttonB;
        comvault->requests.buttonX = packet.requests.buttonX;
        comvault->requests.buttonY = packet.requests.buttonY;
        comvault->unlock();
      break;      
      case PACKET_1:

      break;      
      case PACKET_2:

      break;      
      case PACKET_3:

      break;      
    }
  }  
}
