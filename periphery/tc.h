

//------------------------------------------------------------------------------
/// \unit
///
/// !Purpose
///
/// API for configuring and using Timer Counter (TC) peripherals.
///
/// !Usage
/// -# Optionally, use TC_FindMckDivisor() to let the program find the best
///    TCCLKS field value automatically.
/// -# Configure a Timer Counter in the desired mode using TC_Configure().
/// -# Start or stop the timer clock using TC_Start() and TC_Stop().
//------------------------------------------------------------------------------

#ifndef TC_H
#define TC_H

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include <board.h>

#if !defined(AT91C_ID_TC0) && defined(AT91C_ID_TC012)
    #define AT91C_ID_TC0 AT91C_ID_TC012
#endif

//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------

extern void TC_Configure(AT91S_TC *pTc, unsigned int mode);

extern void TC_Start(AT91S_TC *pTc);

extern void TC_Stop(AT91S_TC *pTc);

extern unsigned char TC_FindMckDivisor(
    unsigned int freq,
    unsigned int mck,
    unsigned int *div,
    unsigned int *tcclks);

#endif //#ifndef TC_H

