#include "async.h"
#include "assert.h"

unsigned char Async::finished(void)
{
  return (this->status != ASYNC_STATUS_PENDING);
}

void Async::setPending(void)
{
  this->status = ASYNC_STATUS_PENDING;
}

void Async::setDone(void)
{
  this->status = ASYNC_STATUS_DONE;
}
