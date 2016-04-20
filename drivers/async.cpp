#include "async.h"

unsigned char ASYNC_IsFinished(Async *pAsync)
{
  return (pAsync->status != ASYNC_STATUS_PENDING);
}
