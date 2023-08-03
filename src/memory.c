//
// The Hook Programming Language
// memory.c
//

#include <hook/memory.h>
#include <stdlib.h>

void *hk_allocate(int size)
{
  return malloc(size);
}

void *hk_reallocate(void *ptr, int size)
{
  return realloc(ptr, size);
}
