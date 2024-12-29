// 
// vm.h
// 
// Copyright 2021 The Hook Programming Language Authors.
// 
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_VM_H
#define HK_VM_H

#include "callable.h"
#include "range.h"
#include "struct.h"
#include "userdata.h"

#define HK_VM_FLAG_NONE     0x00
#define HK_VM_FLAG_NO_TRACE 0x01

#define HK_STACK_MIN_CAPACITY (1 << 8)

#define hk_vm_is_no_trace(s) ((s)->flags & HK_VM_FLAG_NO_TRACE)

#define hk_vm_is_ok(s)    ((s)->status == HK_VM_STATUS_OK)
#define hk_vm_is_exit(s)  ((s)->status == HK_VM_STATUS_EXIT)
#define hk_vm_is_error(s) ((s)->status == HK_VM_STATUS_ERROR)

#define hk_return_if_not_ok(s) do \
  { \
    if (!hk_vm_is_ok(s)) \
      return; \
  } while (0)

typedef enum
{
  HK_VM_STATUS_OK,
  HK_VM_STATUS_EXIT,
  HK_VM_STATUS_ERROR
} HkSateStatus;

typedef struct HkVM
{
  int          stackEnd;
  int          stackTop;
  HkValue      *stackSlots;
  int          flags;
  HkSateStatus status;
} HkVM;

void hk_vm_init(HkVM *vm, int minCapacity);
void hk_vm_deinit(HkVM *vm);
void hk_vm_runtime_error(HkVM *vm, const char *fmt, ...);
void hk_vm_check_argument_type(HkVM *vm, HkValue *args, int index, HkType type);
void hk_vm_check_argument_types(HkVM *vm, HkValue *args, int index, int numTypes, HkType types[]);
void hk_vm_check_argument_bool(HkVM *vm, HkValue *args, int index);
void hk_vm_check_argument_number(HkVM *vm, HkValue *args, int index);
void hk_vm_check_argument_int(HkVM *vm, HkValue *args, int index);
void hk_vm_check_argument_string(HkVM *vm, HkValue *args, int index);
void hk_vm_check_argument_range(HkVM *vm, HkValue *args, int index);
void hk_vm_check_argument_array(HkVM *vm, HkValue *args, int index);
void hk_vm_check_argument_struct(HkVM *vm, HkValue *args, int index);
void hk_vm_check_argument_instance(HkVM *vm, HkValue *args, int index);
void hk_vm_check_argument_iterator(HkVM *vm, HkValue *args, int index);
void hk_vm_check_argument_callable(HkVM *vm, HkValue *args, int index);
void hk_vm_check_argument_userdata(HkVM *vm, HkValue *args, int index);
void hk_vm_push(HkVM *vm, HkValue val);
void hk_vm_push_nil(HkVM *vm);
void hk_vm_push_bool(HkVM *vm, bool data);
void hk_vm_push_number(HkVM *vm, double data);
void hk_vm_push_string(HkVM *vm, HkString *str);
void hk_vm_push_string_from_chars(HkVM *vm, int length, const char *chars);
void hk_vm_push_string_from_stream(HkVM *vm, FILE *stream, const char delim);
void hk_vm_push_range(HkVM *vm, HkRange *range);
void hk_vm_push_array(HkVM *vm, HkArray *arr);
void hk_vm_push_struct(HkVM *vm, HkStruct *ztruct);
void hk_vm_push_instance(HkVM *vm, HkInstance *inst);
void hk_vm_push_iterator(HkVM *vm, HkIterator *it);
void hk_vm_push_closure(HkVM *vm, HkClosure *cl);
void hk_vm_push_native(HkVM *vm, HkNative *native);
void hk_vm_push_new_native(HkVM *vm, const char *name, int arity,
  HkCallFn call);
void hk_vm_push_userdata(HkVM *vm, HkUserdata *udata);
void hk_vm_array(HkVM *vm, int length);
void hk_vm_struct(HkVM *vm, int length);
void hk_vm_instance(HkVM *vm, int numArgs);
void hk_vm_construct(HkVM *vm, int length);
void hk_vm_pop(HkVM *vm);
void hk_vm_call(HkVM *vm, int numArgs);
void hk_vm_compare(HkVM *vm, HkValue val1, HkValue val2, int *result);

#endif // HK_VM_H
