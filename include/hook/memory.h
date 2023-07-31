//
// The Hook Programming Language
// memory.h
//

#ifndef HK_MEMORY_H
#define HK_MEMORY_H

#include <stdint.h>

void *hk_allocate(int size);
void *hk_reallocate(void *ptr, int size);

#endif // HK_MEMORY_H
