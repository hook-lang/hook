//
// The Hook Programming Language
// hk_memory.h
//

#ifndef HK_MEMORY_H
#define HK_MEMORY_H

#include <stdint.h>

void *hk_allocate(int32_t size);
void *hk_reallocate(void *ptr, int32_t size);

#endif // HK_MEMORY_H
