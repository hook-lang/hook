//
// Hook Programming Language
// h_memory.h
//

#ifndef H_MEMORY_H
#define H_MEMORY_H

void *allocate(int size);
void *reallocate(void *ptr, int size);

#endif // H_MEMORY_H
