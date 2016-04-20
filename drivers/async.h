#ifndef ASYNC_H
#define ASYNC_H

#define ASYNC_STATUS_PENDING        0xFF

typedef struct _Async {
  // Asynchronous transfer status
  volatile unsigned char status;
  // Callback function to invoke when transfer completes or fails
  void *callback;
  // Driver storage area; do not use
  unsigned int pStorage[4];
} Async;

extern unsigned char ASYNC_IsFinished(Async *pAsync);

#endif //#ifndef ASYNC_H
