//
// Hook Programming Language
// hook_memory.h
//

#ifndef HOOK_MEMORY_H
#define HOOK_MEMORY_H

void *hk_allocate(int size);
void *hk_reallocate(void *ptr, int size);

#endif // HOOK_MEMORY_H
