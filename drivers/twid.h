#ifndef TWID_H
#define TWID_H

#include "board.h"
#include "async.h"

#define TWID_ERROR_BUSY 1

typedef struct _Twid {
  /// Pointer to the underlying TWI peripheral.
  AT91S_TWI *pTwi;
  /// Current asynchronous transfer being processed.
  Async *pTransfer;
} Twid;

typedef void (*TwiCallback)(Async *);

typedef struct _AsyncTwi {
  // Asynchronous transfer status
  volatile unsigned char status;
  // Callback function to invoke when transfer completes or fails
  TwiCallback callback;
  // Pointer to the data buffer
  unsigned char *pData;
  // Total number of bytes to transfer
  unsigned int num;
  // Number of already transferred bytes
  unsigned int transferred;
} AsyncTwi;

extern void TWID_Initialize(Twid *pTwid,
                            AT91S_TWI *pTwi);

extern void TWID_Handler(Twid *pTwid);

extern unsigned char TWID_Read(Twid *pTwid,
                               unsigned char address,
                               unsigned int iaddress,
                               unsigned char isize,
                               unsigned char *pData,
                               unsigned int num,
                               Async *pAsync);

extern unsigned char TWID_Write(Twid *pTwid,
                                unsigned char address,
                                unsigned int iaddress,
                                unsigned char isize,
                                unsigned char *pData,
                                unsigned int num,
                                Async *pAsync);

void initTWID(void);

#endif //#ifndef TWID_H
