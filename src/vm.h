//
// Hook Programming Language
// vm.h
//

#ifndef VM_H
#define VM_H

#include <stdint.h>
#include "string.h"

#define VM_DEFAULT_NUM_SLOTS 256

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
void vm_push_boolean(vm_t *vm, bool data);
void vm_push_number(vm_t *vm, double data);
void vm_push_string(vm_t *vm, string_t *str);
value_t vm_pop(vm_t *vm);
void vm_execute(vm_t *vm, uint8_t *code, value_t *consts);

#endif
