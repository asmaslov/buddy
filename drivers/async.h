#ifndef ASYNC_H
#define ASYNC_H

#define ASYNC_STATUS_PENDING 0xFF
#define ASYNC_STATUS_DONE 0x00

#define ASYNC_PENDIND(status) ((status & ASYNC_STATUS_PENDING) == ASYNC_STATUS_PENDING)
#define ASYNC_DONE(status) ((status & ASYNC_STATUS_DONE) == ASYNC_STATUS_DONE)

typedef void (*Callback)(void);

struct Async
{
  unsigned char status;
  Callback callback;
  unsigned char finished(void);
  void setPending(void);
  void setDone(void);
};

#endif //#ifndef ASYNC_H
