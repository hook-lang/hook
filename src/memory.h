//
// Hook Programming Language
// memory.h
//

#ifndef MEMORY_H
#define MEMORY_H

void *allocate(int size);
void *reallocate(void *ptr, int size);

#endif // MEMORY_H
