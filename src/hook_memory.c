//
// Hook Programming Language
// hook_memory.c
//

#include "hook_memory.h"
#include <stdlib.h>
#include "hook_error.h"

static inline void check(void *ptr);

static inline void check(void *ptr)
{
  if (!ptr)
    hk_fatal_error("out of memory");
}

void *hk_allocate(int size)
{
  void *ptr = malloc(size);
  check(ptr);
  return ptr;
}

void *hk_reallocate(void *ptr, int size)
{
  ptr = realloc(ptr, size);
  check(ptr);
  return ptr;
}
