//
// Hook Programming Language
// hook_memory.h
//

#ifndef HOOK_MEMORY_H
#define HOOK_MEMORY_H

void *allocate(int size);
void *reallocate(void *ptr, int size);

#endif // HOOK_MEMORY_H
