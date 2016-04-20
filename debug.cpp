#include "debug.h"
#include "dbgu.h"
#include "pio.h"
#include "assert.h"

#include <iostream>
#include <stdarg.h>

// TODO: Make a nice class with DBGU_Init() in constructor
Debug::Debug()
{
  PIO_Configure(DBGU_pins, PIO_LISTSIZE(DBGU_pins));
  TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
}

Debug::~Debug()
{

}

void Debug::printf(char *str, ...)
{
  va_list arg;
  va_start(arg, str);
  char buffer[255];
  //ZeroMemory(&m_lpcBuffer,sizeof(m_lpcBuffer));
  memset(&buffer, 0, sizeof(buffer));
  vsprintf(buffer, str, arg);
  va_end(arg);
  //CharToOem( m_lpcBuffer, m_lpcBuffer);
  printf("%s\n", buffer);
}
