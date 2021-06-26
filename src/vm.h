//
// Hook Programming Language
// vm.h
//

#ifndef VM_H
#define VM_H

#include "value.h"
#include "chunk.h"

#define VM_DEFAULT_NUM_SLOTS 256

#define VM_GET_TOP(vm)    ((vm)->slots[(vm)->index])
#define VM_SET_TOP(vm, v) (vm)->slots[(vm)->index] = (v)

typedef struct
{
  int capacity;
  int end;
  value_t *slots;
  int index;
} vm_t;

void vm_init(vm_t *vm, int min_capacity);
void vm_free(vm_t *vm);
void vm_push_null(vm_t *vm);
void vm_push_number(vm_t *vm, double data);
value_t vm_pop(vm_t *vm);
void vm_execute(vm_t *vm, chunk_t *chunk);

#endif
