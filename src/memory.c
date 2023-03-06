//
// The Hook Programming Language
// memory.c
//

#include <hook/memory.h>
#include <stdlib.h>
#include <hook/error.h>

static inline void check(void *ptr);

static inline void check(void *ptr)
{
  if (!ptr)
    hk_fatal_error("out of memory");
}

void *hk_allocate(int32_t size)
{
  void *ptr = malloc(size);
  check(ptr);
  return ptr;
}

void *hk_reallocate(void *ptr, int32_t size)
{
  ptr = realloc(ptr, size);
  check(ptr);
  return ptr;
}
