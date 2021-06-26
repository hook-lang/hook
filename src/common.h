//
// Hook Programming Language
// common.h
//

#ifndef COMMON_H
#define COMMON_H

#define ASSERT(b, m) do \
  { \
    if (!(b)) \
    { \
      fprintf(stderr, "assertion failed: %s\n", (m)); \
      exit(EXIT_FAILURE); \
    } \
  } while(0)

#endif
