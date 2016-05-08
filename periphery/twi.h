#ifndef TWI_H
#define TWI_H

#include "board.h"

extern void TWI_ConfigureMaster(AT91S_TWI *pTwi,
                                unsigned int twck,
                                unsigned int mck);

extern void TWI_Stop(AT91S_TWI *pTwi);

extern void TWI_StartRead(AT91S_TWI *pTwi,
                          unsigned char address,
                          unsigned int iaddress,
                          unsigned char isize);

extern unsigned char TWI_ReadByte(AT91S_TWI *pTwi);

extern void TWI_StartWrite(AT91S_TWI *pTwi,
                           unsigned char address,
                           unsigned int iaddress,
                           unsigned char isize,
                           unsigned char byte);

extern void TWI_WriteByte(AT91S_TWI *pTwi,
                          unsigned char byte);

extern unsigned char TWI_ByteReceived(AT91S_TWI *pTwi);

extern unsigned char TWI_ByteSent(AT91S_TWI *pTwi);

extern unsigned char TWI_TransferComplete(AT91S_TWI *pTwi);

extern void TWI_EnableIt(AT91S_TWI *pTwi,
                         unsigned int sources);

extern void TWI_DisableIt(AT91S_TWI *pTwi,
                          unsigned int sources);

extern unsigned int TWI_GetStatus(AT91S_TWI *pTwi);

extern unsigned int TWI_GetMaskedStatus(AT91S_TWI *pTwi);

extern void TWI_SendSTOPCondition(AT91S_TWI *pTwi);

#endif //#ifndef TWI_H
