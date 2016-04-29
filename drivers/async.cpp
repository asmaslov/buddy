#include "async.h"
#include "assert.h"

Async::Async()
{
  status = ASYNC_STATUS_DONE;
}

Async::~Async()
{

}

unsigned char Async::finished(void)
{
  return (this->status != ASYNC_STATUS_PENDING);
}

unsigned char Async::busy(void)
{
  return (this->status == ASYNC_STATUS_PENDING);
}

void Async::setPending(void)
{
  this->status = ASYNC_STATUS_PENDING;
}

void Async::setDone(void)
{
  this->status = ASYNC_STATUS_DONE;
}
