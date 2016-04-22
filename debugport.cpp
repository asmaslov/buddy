#include "debugport.h"
#include "dbgu.h"
#include "trace.h"

#include <iostream>
#include <stdarg.h>

DebugPort::DebugPort()
{
  TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
}

DebugPort::~DebugPort()
{

}

void DebugPort::dbgprintf(char *str, ...)
{
  va_list arg;
  va_start(arg, str);
  char buffer[255];
  char i = 0;
  memset(&buffer, 0, sizeof(buffer));
  vsprintf(buffer, str, arg);
  va_end(arg);
  while(buffer[i] != NULL)
  {
    DBGU_PutChar(buffer[i]);
    i++;
  }
}

void DebugPort::dbgputchar(char c)
{
  DBGU_PutChar(c);
}
