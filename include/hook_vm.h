//
// Hook Programming Language
// hook_vm.h
//

#ifndef HOOK_VM_H
#define HOOK_VM_H

#include "hook_range.h"
#include "hook_struct.h"
#include "hook_callable.h"
#include "hook_userdata.h"

#define VM_MIN_CAPACITY (1 << 8)

typedef struct vm
{
  int limit;
  int top;
  value_t *slots;
} vm_t;

void vm_init(vm_t *vm, int min_capacity);
void vm_free(vm_t *vm);
int vm_push(vm_t *vm, value_t val);
int vm_push_nil(vm_t *vm);
int vm_push_boolean(vm_t *vm, bool data);
int vm_push_number(vm_t *vm, double data);
int vm_push_string(vm_t *vm, string_t *str);
int vm_push_string_from_chars(vm_t *vm, int length, const char *chars);
int vm_push_string_from_stream(vm_t *vm, FILE *stream, const char terminal);
int vm_push_range(vm_t *vm, range_t *range);
int vm_push_array(vm_t *vm, array_t *arr);
int vm_push_struct(vm_t *vm, struct_t *ztruct);
int vm_push_instance(vm_t *vm, instance_t *inst);
int vm_push_iterator(vm_t *vm, iterator_t *it);
int vm_push_closure(vm_t *vm, closure_t *cl);
int vm_push_native(vm_t *vm, native_t *native);
int vm_push_new_native(vm_t *vm, const char *name, int arity, int (*call)(vm_t *, value_t *));
int vm_push_userdata(vm_t *vm, userdata_t *udata);
int vm_array(vm_t *vm, int length);
int vm_struct(vm_t *vm, int length);
int vm_instance(vm_t *vm, int length);
int vm_construct(vm_t *vm, int length);
void vm_pop(vm_t *vm);
int vm_call(vm_t *vm, int num_args);
int vm_check_type(value_t *args, int index, type_t type);
int vm_check_types(value_t *args, int index, int num_types, type_t types[]);
int vm_check_boolean(value_t *args, int index);
int vm_check_number(value_t *args, int index);
int vm_check_integer(value_t *args, int index);
int vm_check_int(value_t *args, int index);
int vm_check_string(value_t *args, int index);
int vm_check_range(value_t *args, int index);
int vm_check_array(value_t *args, int index);
int vm_check_struct(value_t *args, int index);
int vm_check_instance(value_t *args, int index);
int vm_check_iterator(value_t *args, int index);
int vm_check_callable(value_t *args, int index);
int vm_check_userdata(value_t *args, int index);

#endif // HOOK_VM_H
