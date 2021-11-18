//
// Hook Programming Language
// common.h
//

#ifndef COMMON_H
#define COMMON_H

#define STATUS_OK       0x00
#define STATUS_ERROR    0x01
#define STATUS_NO_TRACE 0x02

#define ASSERT(cond, msg) do \
  { \
    if (!(cond)) \
    { \
      fprintf(stderr, "assertion failed: %s\n  at %s() in %s:%d", \
        (msg), __func__, __FILE__, __LINE__); \
      exit(EXIT_FAILURE); \
    } \
  } while(0)

#endif // COMMON_H
