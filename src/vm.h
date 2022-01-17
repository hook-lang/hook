//
// Hook Programming Language
// vm.h
//

#ifndef VM_H
#define VM_H

#include "struct.h"
#include "callable.h"
#include "userdata.h"

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
int vm_push_array(vm_t *vm, array_t *arr);
int vm_push_struct(vm_t *vm, struct_t *ztruct);
int vm_push_instance(vm_t *vm, instance_t *inst);
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

#endif // VM_H
