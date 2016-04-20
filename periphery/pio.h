#ifndef PIO_H
#define PIO_H

#include "board.h"

// Pin types
#define PIO_PERIPH_A 0
#define PIO_PERIPH_B 1
#define PIO_INPUT    2
#define PIO_OUTPUT_0 3
#define PIO_OUTPUT_1 4

// Pin attributes
#define PIO_DEFAULT   (0 << 0)
#define PIO_PULLUP    (1 << 0)
#define PIO_DEGLITCH  (1 << 1)
#define PIO_OPENDRAIN (1 << 2)

#define PIO_LISTSIZE(pPins) (sizeof(pPins) / sizeof(Pin))

typedef struct _Pin {
  /// Bitmask indicating which pin(s) to configure.
  unsigned int mask; 
  /// Pointer to the PIO controller which has the pin(s).
  AT91S_PIO *pio;
  /// Peripheral ID of the PIO controller which has the pin(s).
  unsigned char id;
  /// Pin type.
  unsigned char type;
  /// Pin attribute.
  unsigned char attribute;
} Pin;

#ifdef __cplusplus
extern "C" {
#endif
  
  extern unsigned char PIO_Configure(const Pin *list,
                                     unsigned int size);
  
  extern void PIO_Set(const Pin *pin);

  extern void PIO_Clear(const Pin *pin);

  extern unsigned char PIO_Get(const Pin *pin);

  extern unsigned int PIO_GetISR(const Pin *pin);

  extern unsigned char PIO_GetOutputDataStatus(const Pin *pin);
  
#ifdef __cplusplus
}
#endif

#endif //#ifndef PIO_H
