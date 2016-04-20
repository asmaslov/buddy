#ifndef DBGU_H
#define DBGU_H

#include "pio.h"

#define DBGU_STANDARD AT91C_US_PAR_NONE

#define PINS_DBGU {BIT27 | BIT28, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

static const Pin DBGU_pins[] = {
  PINS_DBGU,
};

#ifdef __cplusplus
extern "C" {
#endif

  extern void DBGU_Configure(unsigned int mode,
                             unsigned int baudrate,
                             unsigned int mck);

  extern unsigned char DBGU_GetChar(void);

  extern void DBGU_PutChar(unsigned char c);

  extern unsigned int DBGU_IsRxReady(void);

#ifdef __cplusplus
}
#endif

#endif //#ifndef DBGU_H
