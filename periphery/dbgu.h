#ifndef DBGU_H
#define DBGU_H

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
