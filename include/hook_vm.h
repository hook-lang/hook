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
  int32_t end;
  int32_t top;
  hk_value_t *slots;
} hk_vm_t;

void hk_vm_init(hk_vm_t *vm, int32_t min_capacity);
void hk_vm_free(hk_vm_t *vm);
int32_t hk_vm_push(hk_vm_t *vm, hk_value_t val);
int32_t hk_vm_push_nil(hk_vm_t *vm);
int32_t hk_vm_push_bool(hk_vm_t *vm, bool data);
int32_t hk_vm_push_float(hk_vm_t *vm, double data);
int32_t hk_vm_push_string(hk_vm_t *vm, hk_string_t *str);
int32_t hk_vm_push_string_from_chars(hk_vm_t *vm, int32_t length, const char *chars);
int32_t hk_vm_push_string_from_stream(hk_vm_t *vm, FILE *stream, const char terminal);
int32_t hk_vm_push_range(hk_vm_t *vm, hk_range_t *range);
int32_t hk_vm_push_array(hk_vm_t *vm, hk_array_t *arr);
int32_t hk_vm_push_struct(hk_vm_t *vm, hk_struct_t *ztruct);
int32_t hk_vm_push_instance(hk_vm_t *vm, hk_instance_t *inst);
int32_t hk_vm_push_iterator(hk_vm_t *vm, hk_iterator_t *it);
int32_t hk_vm_push_closure(hk_vm_t *vm, hk_closure_t *cl);
int32_t hk_vm_push_native(hk_vm_t *vm, hk_native_t *native);
int32_t hk_vm_push_new_native(hk_vm_t *vm, const char *name, int32_t arity, int32_t (*call)(hk_vm_t *, hk_value_t *));
int32_t hk_vm_push_userdata(hk_vm_t *vm, hk_userdata_t *udata);
int32_t hk_vm_array(hk_vm_t *vm, int32_t length);
int32_t hk_vm_struct(hk_vm_t *vm, int32_t length);
int32_t hk_vm_instance(hk_vm_t *vm, int32_t length);
int32_t hk_vm_construct(hk_vm_t *vm, int32_t length);
void hk_vm_pop(hk_vm_t *vm);
int32_t hk_vm_call(hk_vm_t *vm, int32_t num_args);
int32_t hk_vm_check_type(hk_value_t *args, int32_t index, int32_t type);
int32_t hk_vm_check_types(hk_value_t *args, int32_t index, int32_t num_types, int32_t types[]);
int32_t hk_vm_check_bool(hk_value_t *args, int32_t index);
int32_t hk_vm_check_float(hk_value_t *args, int32_t index);
int32_t hk_vm_check_integer(hk_value_t *args, int32_t index);
int32_t hk_vm_check_int(hk_value_t *args, int32_t index);
int32_t hk_vm_check_string(hk_value_t *args, int32_t index);
int32_t hk_vm_check_range(hk_value_t *args, int32_t index);
int32_t hk_vm_check_array(hk_value_t *args, int32_t index);
int32_t hk_vm_check_struct(hk_value_t *args, int32_t index);
int32_t hk_vm_check_instance(hk_value_t *args, int32_t index);
int32_t hk_vm_check_iterator(hk_value_t *args, int32_t index);
int32_t hk_vm_check_callable(hk_value_t *args, int32_t index);
int32_t hk_vm_check_userdata(hk_value_t *args, int32_t index);

#endif // HOOK_VM_H
