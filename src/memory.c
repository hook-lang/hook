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

void *hk_allocate(int size)
{
  return malloc(size);
}

void *hk_reallocate(void *ptr, int size)
{
  return realloc(ptr, size);
}
