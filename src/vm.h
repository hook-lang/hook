//
// Hook Programming Language
// vm.h
//

#ifndef VM_H
#define VM_H

#include "stack.h"
#include "chunk.h"

void execute(stack_t *stk, chunk_t *chunk);

#endif
