#include "parser.h"
#include "assert.h"
    
Values *Parser::values;
Holdkeys *Parser::holdkeys;
Requests *Parser::requests;
int *Parser::key;
ControlPacket Parser::packet;
int Parser::nextPartIdx;
bool Parser::packetRcvd;
bool Parser::packetGood;
bool Parser::needFeedback;

Parser::Parser(Values *val, Holdkeys *hk, Requests *rq, int *k)
{
  SANITY_CHECK(val);
  values = val;
  SANITY_CHECK(hk);
  holdkeys = hk;
  SANITY_CHECK(rq);
  requests = rq;
  SANITY_CHECK(k);
  key = k;
  nextPartIdx = 0;
  packetRcvd = false;
  packetGood = false;
  needFeedback = false;  
}

Parser::~Parser()
{

}

void Parser::lock()
{
  *key = 1;
}

void Parser::unlock()
{
  *key = 0;
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
        lock();
        values->leftJoyX = packet.leftJoyXs.val * (packet.leftJoyXs.sign == 0 ? 1 : -1);
        values->leftJoyY = packet.leftJoyYs.val * (packet.leftJoyYs.sign == 0 ? 1 : -1);
        values->rightJoyX = packet.rightJoyXs.val * (packet.rightJoyXs.sign == 0 ? 1 : -1);
        values->rightJoyY = packet.rightJoyYs.val * (packet.rightJoyYs.sign == 0 ? 1 : -1);
        holdkeys->crossUp = packet.codes.crossUp;
        holdkeys->crossDown = packet.codes.crossDown;
        holdkeys->crossLeft = packet.codes.crossLeft;
        holdkeys->crossRight = packet.codes.crossRight;
        requests->buttonA = packet.requests.buttonA;
        requests->buttonB = packet.requests.buttonB;
        requests->buttonX = packet.requests.buttonX;
        requests->buttonY = packet.requests.buttonY;
        unlock();
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
