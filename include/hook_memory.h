//
// Hook Programming Language
// hook_memory.h
//

#ifndef HOOK_MEMORY_H
#define HOOK_MEMORY_H

#include <stdint.h>

void *hk_allocate(int32_t size);
void *hk_reallocate(void *ptr, int32_t size);

#endif // HOOK_MEMORY_H
