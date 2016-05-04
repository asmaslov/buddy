#ifndef TC_H
#define TC_H

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

  extern void TC_Configure(AT91S_TC *pTc, unsigned int mode);

  extern void TC_Start(AT91S_TC *pTc);

  extern void TC_Stop(AT91S_TC *pTc);

  extern unsigned char TC_FindMckDivisor(
      unsigned int freq,
      unsigned int mck,
      unsigned int *div,
      unsigned int *tcclks);

#ifdef __cplusplus
}
#endif

#endif //#ifndef TC_H
