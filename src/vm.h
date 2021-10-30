//
// Hook Programming Language
// vm.h
//

#ifndef VM_H
#define VM_H

#include "struct.h"
#include "callable.h"

#define VM_MIN_CAPACITY 256

typedef struct vm
{
  int capacity;
  int limit;
  value_t *slots;
  int top;
} vm_t;

void vm_init(vm_t *vm, int min_capacity);
void vm_free(vm_t *vm);
int vm_push_value(vm_t *vm, value_t val);
int vm_push_null(vm_t *vm);
int vm_push_boolean(vm_t *vm, bool data);
int vm_push_number(vm_t *vm, double data);
int vm_push_string(vm_t *vm, string_t *str);
int vm_push_array(vm_t *vm, array_t *arr);
int vm_push_struct(vm_t *vm, struct_t *ztruct);
int vm_push_instance(vm_t *vm, instance_t *inst);
int vm_push_function(vm_t *vm, function_t *fn);
int vm_push_native(vm_t *vm, native_t *native);
int vm_push_userdata(vm_t *vm, uint64_t udata);
void vm_pop(vm_t *vm);
void vm_instance(vm_t *vm);
int vm_initialize(vm_t *vm, int num_args);
void vm_compile(vm_t *vm);
int vm_call(vm_t *vm, int num_args);

#endif
