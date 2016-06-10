#ifndef USART_H
#define USART_H

#include "board.h"

// Basic asynchronous mode, i.e. 8 bits no parity.
#define USART_MODE_ASYNCHRONOUS (AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE)
// IRDA mode
#define USART_MODE_IRDA (AT91C_US_USMODE_IRDA | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE | AT91C_US_FILTER)
// All interrupts
#define AT91C_US_ALLIR 0xF3FFF

extern void USART_Configure(AT91S_USART *usart,
                            unsigned int mode,
                            unsigned int baudrate,
                            unsigned int masterClock);

extern void USART_EnableIt(AT91S_USART *usart,
                           unsigned char interrupt);

extern void USART_DisableIt(AT91S_USART *usart,
                            unsigned char interrupt);

extern void USART_SetTransmitterEnabled(AT91S_USART *usart,
                                        unsigned char enabled);

extern void USART_SetReceiverEnabled(AT91S_USART *usart,
                                     unsigned char enabled);

extern void USART_Write(AT91S_USART *usart,
                        unsigned short data, 
                        volatile unsigned int timeout);

extern unsigned char USART_WriteBuffer(AT91S_USART *usart,
                                       void *buffer,
                                       unsigned int size);

extern unsigned short USART_Read(AT91S_USART *usart, 
                                 volatile unsigned int timeout);

extern unsigned char USART_ReadBuffer(AT91S_USART *usart,
                                      void *buffer,
                                      unsigned int size);

extern unsigned char USART_IsDataAvailable(AT91S_USART *usart);

extern void USART_SetIrdaFilter(AT91S_USART *usart,
                                unsigned char filter);

extern void USART_ClearReadBuffers(AT91S_USART *usart);

#endif //#ifndef USART_H
