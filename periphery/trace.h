#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>

#define TRACE_LEVEL_DEBUG    5
#define TRACE_LEVEL_INFO     4
#define TRACE_LEVEL_WARNING  3
#define TRACE_LEVEL_ERROR    2
#define TRACE_LEVEL_FATAL    1
#define TRACE_LEVEL_NO_TRACE 0

// By default, all traces are output except the debug one.
#ifndef TRACE_LEVEL
  #define TRACE_LEVEL TRACE_LEVEL_INFO
#endif

// By default, trace level is static (not dynamic)
#ifndef DYN_TRACES
  #define DYN_TRACES 0
#endif

#ifdef NOTRACE
  #error "Error: NOTRACE has to be not defined !"
#endif

#undef NOTRACE
#if (TRACE_LEVEL == TRACE_LEVEL_NO_TRACE)
  #define NOTRACE
#endif

// Initializes the DBGU
#define TRACE_CONFIGURE(mode, baudrate, mck) \
{ \
  const Pin pinsDbgu[] = {PINS_DBGU}; \
  PIO_Configure(pinsDbgu, PIO_LISTSIZE(pinsDbgu)); \
  DBGU_Configure(mode, baudrate, mck); \
}

// Initializes the DBGU for ISP project
#if (TRACE_LEVEL==0) && (DYNTRACE==0)
  #define TRACE_CONFIGURE_ISP(mode, baudrate, mck) {}
#else
  #define TRACE_CONFIGURE_ISP(mode, baudrate, mck) \
  { \
    const Pin pinsDbgu[] = {PINS_DBGU}; \
    PIO_Configure(pinsDbgu, PIO_LISTSIZE(pinsDbgu)); \
    DBGU_Configure(mode, baudrate, mck); \
  }
#endif

#if defined(NOTRACE)
  // Empty macro
  #define TRACE_DEBUG(...)      { }
  #define TRACE_INFO(...)       { }
  #define TRACE_WARNING(...)    { }               
  #define TRACE_ERROR(...)      { }
  #define TRACE_FATAL(...) \    { while(1); }
  #define TRACE_DEBUG_WP(...)   { }
  #define TRACE_INFO_WP(...)    { }
  #define TRACE_WARNING_WP(...) { }
  #define TRACE_ERROR_WP(...)   { }
  #define TRACE_FATAL_WP(...)   { while(1); }

#elif (DYN_TRACES == 1)
  // Trace output depends on traceLevel value
  #define TRACE_DEBUG(...)      { if (traceLevel >= TRACE_LEVEL_DEBUG)   { printf("-D- " __VA_ARGS__); } }
  #define TRACE_INFO(...)       { if (traceLevel >= TRACE_LEVEL_INFO)    { printf("-I- " __VA_ARGS__); } }
  #define TRACE_WARNING(...)    { if (traceLevel >= TRACE_LEVEL_WARNING) { printf("-W- " __VA_ARGS__); } }
  #define TRACE_ERROR(...)      { if (traceLevel >= TRACE_LEVEL_ERROR)   { printf("-E- " __VA_ARGS__); } }
  #define TRACE_FATAL(...)      { if (traceLevel >= TRACE_LEVEL_FATAL)   { printf("-F- " __VA_ARGS__); while(1); } }
  #define TRACE_DEBUG_WP(...)   { if (traceLevel >= TRACE_LEVEL_DEBUG)   { printf(__VA_ARGS__); } }
  #define TRACE_INFO_WP(...)    { if (traceLevel >= TRACE_LEVEL_INFO)    { printf(__VA_ARGS__); } }
  #define TRACE_WARNING_WP(...) { if (traceLevel >= TRACE_LEVEL_WARNING) { printf(__VA_ARGS__); } }
  #define TRACE_ERROR_WP(...)   { if (traceLevel >= TRACE_LEVEL_ERROR)   { printf(__VA_ARGS__); } }
  #define TRACE_FATAL_WP(...)   { if (traceLevel >= TRACE_LEVEL_FATAL)   { printf(__VA_ARGS__); while(1); } }

#else
  // Trace compilation depends on TRACE_LEVEL value
  #if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
    #define TRACE_DEBUG(...)      { printf("-D- " __VA_ARGS__); }
    #define TRACE_DEBUG_WP(...)   { printf(__VA_ARGS__); }
  #else
    #define TRACE_DEBUG(...)      { }
    #define TRACE_DEBUG_WP(...)   { }
  #endif

  #if (TRACE_LEVEL >= TRACE_LEVEL_INFO)
    #define TRACE_INFO(...)       { printf("-I- " __VA_ARGS__); }
    #define TRACE_INFO_WP(...)    { printf(__VA_ARGS__); }
  #else
    #define TRACE_INFO(...)       { }
    #define TRACE_INFO_WP(...)    { }
  #endif

  #if (TRACE_LEVEL >= TRACE_LEVEL_WARNING)
    #define TRACE_WARNING(...)    { printf("-W- " __VA_ARGS__); }
    #define TRACE_WARNING_WP(...) { printf(__VA_ARGS__); }
  #else
    #define TRACE_WARNING(...)    { }
    #define TRACE_WARNING_WP(...) { }
  #endif

  #if (TRACE_LEVEL >= TRACE_LEVEL_ERROR)
    #define TRACE_ERROR(...)      { printf("-E- " __VA_ARGS__); }
    #define TRACE_ERROR_WP(...)   { printf(__VA_ARGS__); }
  #else
    #define TRACE_ERROR(...)      { }
    #define TRACE_ERROR_WP(...)   { }
  #endif

  #if (TRACE_LEVEL >= TRACE_LEVEL_FATAL)
    #define TRACE_FATAL(...)      { printf("-F- " __VA_ARGS__); while(1); }
    #define TRACE_FATAL_WP(...)   { printf(__VA_ARGS__); while(1); }
  #else
    #define TRACE_FATAL(...)      { while(1); }
    #define TRACE_FATAL_WP(...)   { while(1); }
  #endif

#endif

#if !defined(NOTRACE) && (DYN_TRACES == 1)
  extern unsigned int traceLevel;
#endif

#endif //#ifndef TRACE_H
