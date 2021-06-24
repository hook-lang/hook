//
// Hook Programming Language
// memory.c
//

#include "memory.h"
#include <stdlib.h>
#include "error.h"

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
