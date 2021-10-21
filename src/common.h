//
// Hook Programming Language
// common.h
//

#ifndef COMMON_H
#define COMMON_H

#define STATUS_OK       0
#define STATUS_ERROR    1
#define STATUS_NO_TRACE 2

#define ASSERT(b, m) do \
  { \
    if (!(b)) \
    { \
      fprintf(stderr, "assertion failed: %s\n", (m)); \
      exit(EXIT_FAILURE); \
    } \
  } while(0)

#endif
