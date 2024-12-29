//
// memory.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_MEMORY_H
#define HK_MEMORY_H

#include <stdint.h>

void *hk_allocate(int size);
void *hk_reallocate(void *ptr, int size);

#endif // HK_MEMORY_H
