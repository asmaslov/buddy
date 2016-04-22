#ifndef DEBUGPORT_H
#define DEBUGPORT_H

#include "pio.h"

#define DBGU_STANDARD AT91C_US_PAR_NONE

#define PINS_DBGU {BIT27 | BIT28, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

static const Pin DBGU_pins[] = {
  PINS_DBGU,
};

class DebugPort
{
  public:
    DebugPort();
    ~DebugPort();
    void dbgprintf(char *str, ...);
    void dbgputchar(char c);
};

#endif //#ifndef DEBUGPORT_H
