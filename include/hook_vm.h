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

#define HK_VM_MIN_CAPACITY (1 << 8)

typedef struct hk_vm
{
  int limit;
  int top;
  hk_value_t *slots;
} hk_vm_t;

void hk_vm_init(hk_vm_t *vm, int min_capacity);
void hk_vm_free(hk_vm_t *vm);
int hk_vm_push(hk_vm_t *vm, hk_value_t val);
int hk_vm_push_nil(hk_vm_t *vm);
int hk_vm_push_boolean(hk_vm_t *vm, bool data);
int hk_vm_push_number(hk_vm_t *vm, double data);
int hk_vm_push_string(hk_vm_t *vm, hk_string_t *str);
int hk_vm_push_string_from_chars(hk_vm_t *vm, int length, const char *chars);
int hk_vm_push_string_from_stream(hk_vm_t *vm, FILE *stream, const char terminal);
int hk_vm_push_range(hk_vm_t *vm, hk_range_t *range);
int hk_vm_push_array(hk_vm_t *vm, hk_array_t *arr);
int hk_vm_push_struct(hk_vm_t *vm, hk_struct_t *ztruct);
int hk_vm_push_instance(hk_vm_t *vm, hk_instance_t *inst);
int hk_vm_push_iterator(hk_vm_t *vm, hk_iterator_t *it);
int hk_vm_push_closure(hk_vm_t *vm, hk_closure_t *cl);
int hk_vm_push_native(hk_vm_t *vm, hk_native_t *native);
int hk_vm_push_new_native(hk_vm_t *vm, const char *name, int arity, int (*call)(hk_vm_t *, hk_value_t *));
int hk_vm_push_userdata(hk_vm_t *vm, hk_userdata_t *udata);
int hk_vm_array(hk_vm_t *vm, int length);
int hk_vm_struct(hk_vm_t *vm, int length);
int hk_vm_instance(hk_vm_t *vm, int length);
int hk_vm_construct(hk_vm_t *vm, int length);
void hk_vm_pop(hk_vm_t *vm);
int hk_vm_call(hk_vm_t *vm, int num_args);
int hk_vm_check_type(hk_value_t *args, int index, int type);
int hk_vm_check_types(hk_value_t *args, int index, int num_types, int types[]);
int hk_vm_check_boolean(hk_value_t *args, int index);
int hk_vm_check_number(hk_value_t *args, int index);
int hk_vm_check_integer(hk_value_t *args, int index);
int hk_vm_check_int(hk_value_t *args, int index);
int hk_vm_check_string(hk_value_t *args, int index);
int hk_vm_check_range(hk_value_t *args, int index);
int hk_vm_check_array(hk_value_t *args, int index);
int hk_vm_check_struct(hk_value_t *args, int index);
int hk_vm_check_instance(hk_value_t *args, int index);
int hk_vm_check_iterator(hk_value_t *args, int index);
int hk_vm_check_callable(hk_value_t *args, int index);
int hk_vm_check_userdata(hk_value_t *args, int index);

#endif // HOOK_VM_H
