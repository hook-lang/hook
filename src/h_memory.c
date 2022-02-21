//
// Hook Programming Language
// h_memory.c
//

#include "h_memory.h"
#include <stdlib.h>
#include "h_error.h"

static inline void check(void *ptr);

static inline void check(void *ptr)
{
  if (!ptr)
    fatal_error("out of memory");
}

void *allocate(int size)
{
  void *ptr = malloc(size);
  check(ptr);
  return ptr;
}

void *reallocate(void *ptr, int size)
{
  ptr = realloc(ptr, size);
  check(ptr);
  return ptr;
}
