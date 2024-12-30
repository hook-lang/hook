//
// memory.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include <hook/memory.h>
#include <stdlib.h>

void *hk_allocate(size_t size)
{
  return malloc(size);
}

void *hk_reallocate(void *ptr, size_t size)
{
  return realloc(ptr, size);
}

void hk_free(void *ptr)
{
  free(ptr);
}
