#ifndef ASSERT_H
#define ASSERT_H

#include "trace.h"

#ifdef NOASSERT
  #define ASSERT(...)
  #define SANITY_CHECK(...)
#else
  #if (TRACE_LEVEL == 0)
    // Checks that the given condition is true, otherwise stops the program execution
    #define ASSERT(condition, ...) \
    { \
      if (!(condition)) \
      { \
        while (1); \
      } \
    }
    
    // Performs the same duty as the ASSERT() macro
    #define SANITY_CHECK(condition) ASSERT(condition, ...)

  #else
    // Checks that the given condition is true, otherwise displays an error message and stops the program execution
    #define ASSERT(condition, ...) \
    { \
      if (!(condition)) \
      { \
        printf("-F- ASSERT: "); \
        printf(__VA_ARGS__); \
        while (1); \
      } \
    }
    
    #define SANITY_ERROR "Sanity check failed at %s:%d\n\r"
    
    // Performs the same duty as the ASSERT() macro, except a default error message is output if the condition is false
    #define SANITY_CHECK(condition) ASSERT(condition, SANITY_ERROR, __FILE__, __LINE__)

  #endif
#endif

#endif //#ifndef ASSERT_H
