#ifndef DBGU_H
#define DBGU_H

extern void DBGU_Configure(unsigned int mode,
                           unsigned int baudrate,
                           unsigned int mck);

extern void DBGU_PutChar(unsigned char c);

extern unsigned int DBGU_IsRxReady(void);

extern unsigned char DBGU_GetChar(void);

#endif //#ifndef DBGU_H
