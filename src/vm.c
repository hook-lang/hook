// 
// vm.c
// 
// Copyright 2021 The Hook Programming Language Authors.
// 
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include <hook/vm.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <hook/struct.h>
#include <hook/iterable.h>
#include <hook/memory.h>
#include <hook/utils.h>
#include "builtin.h"
#include "module.h"

static inline void type_error(HkVM *vm, int index, int numTypes, HkType types[],
  HkType valType);
static inline void push(HkVM *vm, HkValue val);
static inline void pop(HkVM *vm);
static inline int read_byte(uint8_t **pc);
static inline int read_word(uint8_t **pc);
static inline void do_range(HkVM *vm);
static inline void do_array(HkVM *vm, int length);
static inline void do_struct(HkVM *vm, int length);
static inline void do_instance(HkVM *vm, int numArgs);
static inline void adjust_instance_args(HkVM *vm, int length, int numArgs);
static inline void do_construct(HkVM *vm, int length);
static inline void do_iterator(HkVM *vm);
static inline void do_closure(HkVM *vm, HkFunction *fn);
static inline void do_unpack_array(HkVM *vm, int n);
static inline void do_unpack_struct(HkVM *vm, int n);
static inline void do_add_element(HkVM *vm);
static inline void do_get_element(HkVM *vm);
static inline void slice_string(HkVM *vm, HkValue *slot, HkString *str, HkRange *range);
static inline void slice_array(HkVM *vm, HkValue *slot, HkArray *arr, HkRange *range);
static inline void do_fetch_element(HkVM *vm);
static inline void do_set_element(HkVM *vm);
static inline void do_put_element(HkVM *vm);
static inline void do_delete_element(HkVM *vm);
static inline void do_inplace_add_element(HkVM *vm);
static inline void do_inplace_put_element(HkVM *vm);
static inline void do_inplace_delete_element(HkVM *vm);
static inline void do_get_field(HkVM *vm, HkString *name);
static inline void do_fetch_field(HkVM *vm, HkString *name);
static inline void do_set_field(HkVM *vm);
static inline void do_put_field(HkVM *vm, HkString *name);
static inline void do_inplace_put_field(HkVM *vm, HkString *name);
static inline void do_current(HkVM *vm);
static inline void do_next(HkVM *vm);
static inline void do_equal(HkVM *vm);
static inline void do_greater(HkVM *vm);
static inline void do_less(HkVM *vm);
static inline void do_not_equal(HkVM *vm);
static inline void do_not_greater(HkVM *vm);
static inline void do_not_less(HkVM *vm);
static inline void do_bitwise_or(HkVM *vm);
static inline void do_bitwise_xor(HkVM *vm);
static inline void do_bitwise_and(HkVM *vm);
static inline void do_left_shift(HkVM *vm);
static inline void do_right_shift(HkVM *vm);
static inline void do_add(HkVM *vm);
static inline void concat_strings(HkVM *vm, HkValue *slots, HkValue val1, HkValue val2);
static inline void concat_arrays(HkVM *vm, HkValue *slots, HkValue val1, HkValue val2);
static inline void do_subtract(HkVM *vm);
static inline void diff_arrays(HkVM *vm, HkValue *slots, HkValue val1, HkValue val2);
static inline void do_multiply(HkVM *vm);
static inline void do_divide(HkVM *vm);
static inline void do_quotient(HkVM *vm);
static inline void do_remainder(HkVM *vm);
static inline void do_negate(HkVM *vm);
static inline void do_not(HkVM *vm);
static inline void do_bitwise_not(HkVM *vm);
static inline void do_increment(HkVM *vm);
static inline void do_decrement(HkVM *vm);
static inline void do_call(HkVM *vm, int numArgs);
static inline void adjust_call_args(HkVM *vm, int arity, int numArgs);
static inline void print_trace(HkString *name, HkString *file, int line);
static inline void call_function(HkVM *vm, HkValue *locals, HkClosure *cl, int *line);
static inline void discard_frame(HkVM *vm, HkValue *slots);
static inline void move_result(HkVM *vm, HkValue *slots);

static inline void type_error(HkVM *vm, int index, int numTypes, HkType types[],
  HkType valType)
{
  hk_assert(numTypes > 0, "numTypes must be greater than 0");
  vm->status = HK_VM_STATUS_ERROR;
  fprintf(stderr, "runtime error: type error: argument #%d must be of the type %s",
    index, hk_type_name(types[0]));
  for (int i = 1; i < numTypes; ++i)
    fprintf(stderr, "|%s", hk_type_name(types[i]));
  fprintf(stderr, ", %s given\n", hk_type_name(valType));
}

static inline void push(HkVM *vm, HkValue val)
{
  if (vm->stackTop == vm->stackEnd)
  {
    hk_vm_runtime_error(vm, "stack overflow");
    return;
  }
  ++vm->stackTop;
  vm->stackSlots[vm->stackTop] = val;
}

static inline void pop(HkVM *vm)
{
  hk_assert(vm->stackTop > -1, "stack underflow");
  HkValue val = vm->stackSlots[vm->stackTop];
  --vm->stackTop;
  hk_value_release(val);
}

static inline int read_byte(uint8_t **pc)
{
  int byte = **pc;
  ++(*pc);
  return byte;
}

static inline int read_word(uint8_t **pc)
{
  int word = *((uint16_t *) *pc);
  *pc += 2;
  return word;
}

static inline void do_range(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_vm_runtime_error(vm, "type error: range must be of type number");
    return;
  }
  HkRange *range = hk_range_new((int64_t) hk_as_number(val1), (int64_t) hk_as_number(val2));
  hk_incr_ref(range);
  slots[0] = hk_range_value(range);
  --vm->stackTop;
}

static inline void do_array(HkVM *vm, int length)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - length + 1];
  HkArray *arr = hk_array_new_with_capacity(length);
  arr->length = length;
  for (int i = 0; i < length; ++i)
    arr->elements[i] = slots[i];
  vm->stackTop -= length;
  push(vm, hk_array_value(arr));
  if (!hk_vm_is_ok(vm))
  {
    hk_array_free(arr);
    return;
  }
  hk_incr_ref(arr);
}

static inline void do_struct(HkVM *vm, int length)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - length];
  HkValue val = slots[0];
  HkString *struct_name = hk_is_nil(val) ? NULL : hk_as_string(val);
  HkStruct *ztruct = hk_struct_new(struct_name);
  for (int i = 1; i <= length; ++i)
  {
    HkString *field_name = hk_as_string(slots[i]);
    if (!hk_struct_define_field(ztruct, field_name))
    {
      hk_vm_runtime_error(vm, "field %.*s is already defined", field_name->length,
        field_name->chars);
      hk_struct_free(ztruct);
      return;
    }
  }
  for (int i = 1; i <= length; ++i)
    hk_decr_ref(hk_as_object(slots[i]));
  vm->stackTop -= length;
  hk_incr_ref(ztruct);
  slots[0] = hk_struct_value(ztruct);
  if (struct_name)
    hk_decr_ref(struct_name);
}

static inline void do_instance(HkVM *vm, int numArgs)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - numArgs];
  HkValue val = slots[0];
  if (!hk_is_struct(val))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as a struct", hk_type_name(val.type));
    return;
  }
  HkStruct *ztruct = hk_as_struct(val);
  int length = ztruct->length;
  adjust_instance_args(vm, length, numArgs);
  HkInstance *inst = hk_instance_new(ztruct);
  for (int i = 0; i < length; ++i)
    inst->values[i] = slots[i + 1];
  vm->stackTop -= length;
  hk_incr_ref(inst);
  slots[0] = hk_instance_value(inst);
  hk_struct_release(ztruct);
}

static inline void adjust_instance_args(HkVM *vm, int length, int numArgs)
{
  if (numArgs > length)
  {
    do
    {
      pop(vm);
      --numArgs;
    }
    while (numArgs > length);
    return;
  }
  while (numArgs < length)
  {
    push(vm, HK_NIL_VALUE);
    hk_return_if_not_ok(vm);
    ++numArgs;
  }
}

static inline void do_construct(HkVM *vm, int length)
{
  int n = length << 1;
  HkValue *slots = &vm->stackSlots[vm->stackTop - n];
  HkValue val = slots[0];
  HkString *struct_name = hk_is_nil(val) ? NULL : hk_as_string(val);
  HkStruct *ztruct = hk_struct_new(struct_name);
  for (int i = 1; i <= n; i += 2)
  {
    HkString *field_name = hk_as_string(slots[i]);
    if (hk_struct_define_field(ztruct, field_name))
      continue;
    hk_vm_runtime_error(vm, "field %.*s is already defined", field_name->length,
      field_name->chars);
    hk_struct_free(ztruct);
    return;
  }
  for (int i = 1; i <= n; i += 2)
    hk_decr_ref(hk_as_object(slots[i]));
  HkInstance *inst = hk_instance_new(ztruct);
  for (int i = 2, j = 0; i <= n + 1; i += 2, ++j)
    inst->values[j] = slots[i];
  vm->stackTop -= n;
  hk_incr_ref(inst);
  slots[0] = hk_instance_value(inst);
  if (struct_name)
    hk_decr_ref(struct_name);
}

static inline void do_iterator(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop];
  HkValue val = slots[0];
  if (hk_is_iterator(val))
    return;
  HkIterator *it = hk_new_iterator(val);
  if (!it)
  {
    hk_vm_runtime_error(vm, "type error: value of type %s is not iterable", hk_type_name(val.type));
    return;
  }
  hk_incr_ref(it);
  slots[0] = hk_iterator_value(it);
  hk_value_release(val);
}

static inline void do_closure(HkVM *vm, HkFunction *fn)
{
  int numNonlocals = fn->numNonlocals;
  HkValue *slots = &vm->stackSlots[vm->stackTop - numNonlocals + 1];
  HkClosure *cl = hk_closure_new(fn);
  for (int i = 0; i < numNonlocals; ++i)
    cl->nonlocals[i] = slots[i];
  vm->stackTop -= numNonlocals;
  push(vm, hk_closure_value(cl));
  if (!hk_vm_is_ok(vm))
  {
    hk_closure_free(cl);
    return;
  }
  hk_incr_ref(cl);
}

static inline void do_unpack_array(HkVM *vm, int n)
{
  HkValue val = vm->stackSlots[vm->stackTop];
  if (!hk_is_array(val))
  {
    hk_vm_runtime_error(vm, "type error: value of type %s is not an array",
      hk_type_name(val.type));
    return;
  }
  HkArray *arr = hk_as_array(val);
  --vm->stackTop;
  for (int i = 0; i < n && i < arr->length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    push(vm, elem);
    if (!hk_vm_is_ok(vm))
      goto end;
    hk_value_incr_ref(elem);
  }
  for (int i = arr->length; i < n; ++i)
  {
    push(vm, HK_NIL_VALUE);
    if (!hk_vm_is_ok(vm))
      break;
  }
end:
  hk_array_release(arr);
}

static inline void do_unpack_struct(HkVM *vm, int n)
{
  HkValue val = vm->stackSlots[vm->stackTop];
  if (!hk_is_instance(val))
  {
    hk_vm_runtime_error(vm, "type error: value of type %s is not an instance of struct",
      hk_type_name(val.type));
    return;
  }
  HkInstance *inst = hk_as_instance(val);
  HkStruct *ztruct = inst->ztruct;
  HkValue *slots = &vm->stackSlots[vm->stackTop - n];
  for (int i = 0; i < n; ++i)
  {
    HkString *name = hk_as_string(slots[i]);
    int index = hk_struct_index_of(ztruct, name);
    HkValue value = index == -1 ? HK_NIL_VALUE :
      hk_instance_get_field(inst, index);
    hk_value_incr_ref(value);
    hk_decr_ref(name);
    slots[i] = value;
  }
  --vm->stackTop;
  hk_instance_release(inst);
}

static inline void do_add_element(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  HkArray *result = hk_array_add_element(arr, val2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stackTop;
  hk_array_release(arr);  
  hk_value_decr_ref(val2);
}

static inline void do_get_element(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (hk_is_string(val1))
  {
    HkString *str = hk_as_string(val1);
    if (hk_is_int(val2))
    {
      int64_t index = (int64_t) hk_as_number(val2);
      if (index < 0 || index >= str->length)
      {
        hk_vm_runtime_error(vm, "range error: index %d is out of bounds for string of length %d",
          index, str->length);
        return;
      }
      HkValue result = hk_string_value(hk_string_from_chars(1, &str->chars[(int) index]));
      hk_value_incr_ref(result);
      slots[0] = result;
      --vm->stackTop;
      hk_string_release(str);
      return;
    }
    if (!hk_is_range(val2))
    {
      hk_vm_runtime_error(vm, "type error: string cannot be indexed by %s", hk_type_name(val2.type));
      return;
    }
    slice_string(vm, slots, str, hk_as_range(val2));
    return;
  }
  if (!hk_is_array(val1))
  {
    hk_vm_runtime_error(vm, "type error: %s cannot be indexed", hk_type_name(val1.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  if (hk_is_int(val2))
  {
    int64_t index = (int64_t) hk_as_number(val2);
    if (index < 0 || index >= arr->length)
    {
      hk_vm_runtime_error(vm, "range error: index %d is out of bounds for array of length %d",
        index, arr->length);
      return;
    }
    HkValue result = hk_array_get_element(arr, (int) index);
    hk_value_incr_ref(result);
    slots[0] = result;
    --vm->stackTop;
    hk_array_release(arr);
    return;
  }
  if (!hk_is_range(val2))
  {
    hk_vm_runtime_error(vm, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  slice_array(vm, slots, arr, hk_as_range(val2));
}

static inline void slice_string(HkVM *vm, HkValue *slot, HkString *str, HkRange *range)
{
  int str_end = str->length - 1;
  int64_t start = range->start;
  int64_t end = range->end;
  HkString *result;
  if (start > end || start > str_end || end < 0)
  {
    result = hk_string_new();
    goto end;
  }
  if (start <= 0 && end >= str_end)
  {
    --vm->stackTop;
    hk_range_release(range);
    return;
  }
  int length = (int) (end - start + 1);
  result = hk_string_from_chars(length, &str->chars[start]);
end:
  hk_incr_ref(result);
  *slot = hk_string_value(result);
  --vm->stackTop;
  hk_string_release(str);
  hk_range_release(range);
}

static inline void slice_array(HkVM *vm, HkValue *slot, HkArray *arr, HkRange *range)
{
  int arr_end = arr->length - 1;
  int64_t start = range->start;
  int64_t end = range->end;
  HkArray *result;
  if (start > end || start > arr_end || end < 0)
  {
    result = hk_array_new();
    goto end;
  }
  if (start <= 0 && end >= arr_end)
  {
    --vm->stackTop;
    hk_range_release(range);
    return;
  }
  int length = (int) (end - start + 1);
  result = hk_array_new_with_capacity(length);
  result->length = length;
  for (int64_t i = start, j = 0; i <= end ; ++i, ++j)
  {
    HkValue elem = hk_array_get_element(arr, (int) i);
    hk_value_incr_ref(elem);
    result->elements[(int) j] = elem;
  }
end:
  hk_incr_ref(result);
  *slot = hk_array_value(result);
  --vm->stackTop;
  hk_array_release(arr);
  hk_range_release(range);
}

static inline void do_fetch_element(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  if (!hk_is_int(val2))
  {
    hk_vm_runtime_error(vm, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_vm_runtime_error(vm, "range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return;
  }
  HkValue elem = hk_array_get_element(arr, (int) index);
  push(vm, elem);
  hk_return_if_not_ok(vm);
  hk_value_incr_ref(elem);
}

static inline void do_set_element(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 2];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  HkValue val3 = slots[2];
  HkArray *arr = hk_as_array(val1);
  int index = (int) hk_as_number(val2);
  HkArray *result = hk_array_set_element(arr, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  vm->stackTop -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
}

static inline void do_put_element(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 2];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  HkValue val3 = slots[2];
  if (!hk_is_array(val1))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  if (!hk_is_int(val2))
  {
    hk_vm_runtime_error(vm, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_vm_runtime_error(vm, "range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return;
  }
  HkArray *result = hk_array_set_element(arr, (int) index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  vm->stackTop -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
}

static inline void do_delete_element(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  if (!hk_is_int(val2))
  {
    hk_vm_runtime_error(vm, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_vm_runtime_error(vm, "range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return;
  }
  HkArray *result = hk_array_delete_element(arr, (int) index);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stackTop;
  hk_array_release(arr);
}

static inline void do_inplace_add_element(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  if (arr->refCount == 2)
  {
    hk_array_inplace_add_element(arr, val2);
    --vm->stackTop;
    hk_value_decr_ref(val2);
    return;
  }
  HkArray *result = hk_array_add_element(arr, val2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stackTop;
  hk_array_release(arr);
  hk_value_decr_ref(val2);
}

static inline void do_inplace_put_element(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 2];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  HkValue val3 = slots[2];
  if (!hk_is_array(val1))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  if (!hk_is_int(val2))
  {
    hk_vm_runtime_error(vm, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_vm_runtime_error(vm, "range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return;
  }
  if (arr->refCount == 2)
  {
    hk_array_inplace_set_element(arr, (int) index, val3);
    vm->stackTop -= 2;
    hk_value_decr_ref(val3);
    return;
  }
  HkArray *result = hk_array_set_element(arr, (int) index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  vm->stackTop -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
}

static inline void do_inplace_delete_element(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  if (!hk_is_int(val2))
  {
    hk_vm_runtime_error(vm, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_vm_runtime_error(vm, "range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return;
  }
  if (arr->refCount == 2)
  {
    hk_array_inplace_delete_element(arr, (int) index);
    --vm->stackTop;
    return;
  }
  HkArray *result = hk_array_delete_element(arr, (int) index);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stackTop;
  hk_array_release(arr);
}

static inline void do_get_field(HkVM *vm, HkString *name)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop];
  HkValue val = slots[0];
  if (!hk_is_instance(val))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as an instance of struct",
      hk_type_name(val.type));
    return;
  }
  HkInstance *inst = hk_as_instance(val);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_vm_runtime_error(vm, "no field %.*s on struct", name->length, name->chars);
    return;
  }
  HkValue value = hk_instance_get_field(inst, index);
  hk_value_incr_ref(value);
  slots[0] = value;
  hk_instance_release(inst);
}

static inline void do_fetch_field(HkVM *vm, HkString *name)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop];
  HkValue val = slots[0];
  if (!hk_is_instance(val))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as an instance of struct",
      hk_type_name(val.type));
    return;
  }
  HkInstance *inst = hk_as_instance(val);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_vm_runtime_error(vm, "no field %.*s on struct", name->length, name->chars);
    return;
  }
  push(vm, hk_number_value(index));
  hk_return_if_not_ok(vm);
  HkValue value = hk_instance_get_field(inst, index);
  push(vm, value);
  hk_return_if_not_ok(vm);
  hk_value_incr_ref(value);
}

static inline void do_set_field(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 2];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  HkValue val3 = slots[2];
  HkInstance *inst = hk_as_instance(val1);
  int index = (int) hk_as_number(val2);
  HkInstance *result = hk_instance_set_field(inst, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  vm->stackTop -= 2;
  hk_instance_release(inst);
  hk_value_decr_ref(val3);
}

static inline void do_put_field(HkVM *vm, HkString *name)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_instance(val1))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as an instance of struct",
      hk_type_name(val1.type));
    return;
  }
  HkInstance *inst = hk_as_instance(val1);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_vm_runtime_error(vm, "no field %.*s on struct", name->length, name->chars);
    return;
  }
  HkInstance *result = hk_instance_set_field(inst, index, val2);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  --vm->stackTop;
  hk_instance_release(inst);
  hk_value_decr_ref(val2);
}

static inline void do_inplace_put_field(HkVM *vm, HkString *name)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_instance(val1))
  {
    hk_vm_runtime_error(vm, "type error: cannot use %s as an instance of struct",
      hk_type_name(val1.type));
    return;
  }
  HkInstance *inst = hk_as_instance(val1);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_vm_runtime_error(vm, "no field %.*s on struct", name->length, name->chars);
    return;
  }
  if (inst->refCount == 2)
  {
    hk_instance_inplace_set_field(inst, index, val2);
    --vm->stackTop;
    hk_value_decr_ref(val2);
    return;
  }
  HkInstance *result = hk_instance_set_field(inst, index, val2);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  --vm->stackTop;
  hk_instance_release(inst);
  hk_value_decr_ref(val2);
}

static inline void do_current(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val = slots[1];
  HkIterator *it = hk_as_iterator(val);
  HkValue result = hk_iterator_get_current(it);
  hk_value_incr_ref(result);
  hk_value_release(slots[0]);
  slots[0] = result;
}

static inline void do_next(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop];
  HkValue val = slots[0];
  HkIterator *it = hk_as_iterator(val);
  if (it->refCount == 2)
  {
    hk_iterator_inplace_next(it);
    return;
  }
  HkIterator *result = hk_iterator_next(it);
  hk_incr_ref(result);
  slots[0] = hk_iterator_value(result);
  hk_iterator_release(it);
}

static inline void do_equal(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  slots[0] = hk_value_equal(val1, val2) ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --vm->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline void do_greater(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  hk_vm_compare(vm, val1, val2, &result);
  hk_return_if_not_ok(vm);
  slots[0] = result > 0 ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --vm->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline void do_less(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  hk_vm_compare(vm, val1, val2, &result);
  hk_return_if_not_ok(vm);
  slots[0] = result < 0 ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --vm->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline void do_not_equal(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  slots[0] = hk_value_equal(val1, val2) ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --vm->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline void do_not_greater(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  hk_vm_compare(vm, val1, val2, &result);
  hk_return_if_not_ok(vm);
  slots[0] = result > 0 ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --vm->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline void do_not_less(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  hk_vm_compare(vm, val1, val2, &result);
  hk_return_if_not_ok(vm);
  slots[0] = result < 0 ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --vm->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline void do_bitwise_or(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_vm_runtime_error(vm, "type error: cannot apply `bitwise or` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  double data = (double) (((int64_t) hk_as_number(val1)) | ((int64_t) hk_as_number(val2)));
  slots[0] = hk_number_value(data);
  --vm->stackTop;
}

static inline void do_bitwise_xor(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_vm_runtime_error(vm, "type error: cannot apply `bitwise xor` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  double data = (double) (((int64_t) hk_as_number(val1)) ^ ((int64_t) hk_as_number(val2)));
  slots[0] = hk_number_value(data);
  --vm->stackTop;
}

static inline void do_bitwise_and(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_vm_runtime_error(vm, "type error: cannot apply `bitwise and` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  double data = (double) (((int64_t) hk_as_number(val1)) & ((int64_t) hk_as_number(val2)));
  slots[0] = hk_number_value(data);
  --vm->stackTop;
}

static inline void do_left_shift(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_vm_runtime_error(vm, "type error: cannot apply `left shift` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  double data = (double) (((int64_t) hk_as_number(val1)) << ((int64_t) hk_as_number(val2)));
  slots[0] = hk_number_value(data);
  --vm->stackTop;
}

static inline void do_right_shift(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_vm_runtime_error(vm, "type error: cannot apply `right shift` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  double data = (double) (((int64_t) hk_as_number(val1)) >> ((int64_t) hk_as_number(val2)));
  slots[0] = hk_number_value(data);
  --vm->stackTop;
}

static inline void do_add(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (hk_is_number(val1))
  {
    if (!hk_is_number(val2))
    {
      hk_vm_runtime_error(vm, "type error: cannot add %s to number", hk_type_name(val2.type));
      return;
    }
    double data = hk_as_number(val1) + hk_as_number(val2);
    slots[0] = hk_number_value(data);
    --vm->stackTop;
    return;
  }
  if (hk_is_string(val1))
  {
    if (!hk_is_string(val2))
    {
      hk_vm_runtime_error(vm, "type error: cannot concatenate string and %s",
        hk_type_name(val2.type));
      return;
    }
    concat_strings(vm, slots, val1, val2);
    return;
  }
  if (hk_is_array(val1))
  {
    if (!hk_is_array(val2))
    {
      hk_vm_runtime_error(vm, "type error: cannot concatenate array and %s",
        hk_type_name(val2.type));
      return;
    }
    concat_arrays(vm, slots, val1, val2);
    return;
  }
  hk_vm_runtime_error(vm, "type error: cannot add %s to %s", hk_type_name(val2.type),
    hk_type_name(val1.type));
}

static inline void concat_strings(HkVM *vm, HkValue *slots, HkValue val1, HkValue val2)
{
  HkString *str1 = hk_as_string(val1);
  if (!str1->length)
  {
    slots[0] = val2;
    --vm->stackTop;
    hk_string_release(str1);
    return;
  }
  HkString *str2 = hk_as_string(val2);
  if (!str2->length)
  {
    --vm->stackTop;
    hk_string_release(str2);
    return;
  }
  if (str1->refCount == 1)
  {
    hk_string_inplace_concat(str1, str2);
    --vm->stackTop;
    hk_string_release(str2);
    return;
  }
  HkString *result = hk_string_concat(str1, str2);
  hk_incr_ref(result);
  slots[0] = hk_string_value(result);
  --vm->stackTop;
  hk_string_release(str1);
  hk_string_release(str2);
}

static inline void concat_arrays(HkVM *vm, HkValue *slots, HkValue val1, HkValue val2)
{
  HkArray *arr1 = hk_as_array(val1);
  if (!arr1->length)
  {
    slots[0] = val2;
    --vm->stackTop;
    hk_array_release(arr1);
    return;
  }
  HkArray *arr2 = hk_as_array(val2);
  if (!arr2->length)
  {
    --vm->stackTop;
    hk_array_release(arr2);
    return;
  }
  if (arr1->refCount == 1)
  {
    hk_array_inplace_concat(arr1, arr2);
    --vm->stackTop;
    hk_array_release(arr2);
    return;
  }
  HkArray *result = hk_array_concat(arr1, arr2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stackTop;
  hk_array_release(arr1);
  hk_array_release(arr2);
}

static inline void do_subtract(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (hk_is_number(val1))
  {
    if (!hk_is_number(val2))
    {
      hk_vm_runtime_error(vm, "type error: cannot subtract %s from number",
        hk_type_name(val2.type));
      return;
    }
    double data = hk_as_number(val1) - hk_as_number(val2);
    slots[0] = hk_number_value(data);
    --vm->stackTop;
    return;
  }
  if (hk_is_array(val1))
  {
    if (!hk_is_array(val2))
    {
      hk_vm_runtime_error(vm, "type error: cannot diff between array and %s",
        hk_type_name(val2.type));
      return;
    }
    diff_arrays(vm, slots, val1, val2);
    return;
  }
  hk_vm_runtime_error(vm, "type error: cannot subtract %s from %s", hk_type_name(val2.type),
    hk_type_name(val1.type));
}

static inline void diff_arrays(HkVM *vm, HkValue *slots, HkValue val1, HkValue val2)
{
  HkArray *arr1 = hk_as_array(val1);
  HkArray *arr2 = hk_as_array(val2);
  if (!arr1->length || !arr2->length)
  {
    --vm->stackTop;
    hk_array_release(arr2);
    return;
  }
  if (arr1->refCount == 1)
  {
    hk_array_inplace_diff(arr1, arr2);
    --vm->stackTop;
    hk_array_release(arr2);
    return;
  }
  HkArray *result = hk_array_diff(arr1, arr2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stackTop;
  hk_array_release(arr1);
  hk_array_release(arr2);
}

static inline void do_multiply(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_vm_runtime_error(vm, "type error: cannot multiply %s to %s", hk_type_name(val2.type),
      hk_type_name(val1.type));
    return;
  }
  double data = hk_as_number(val1) * hk_as_number(val2);
  slots[0] = hk_number_value(data);
  --vm->stackTop;
}

static inline void do_divide(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_vm_runtime_error(vm, "type error: cannot divide %s by %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  double data = hk_as_number(val1) / hk_as_number(val2);
  slots[0] = hk_number_value(data);
  --vm->stackTop;
}

static inline void do_quotient(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_vm_runtime_error(vm, "type error: cannot apply `quotient` between %s and %s",
      hk_type_name(val1.type), hk_type_name(val2.type));
    return;
  }
  double data = floor(hk_as_number(val1) / hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --vm->stackTop;
}

static inline void do_remainder(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_vm_runtime_error(vm, "type error: cannot apply `remainder` between %s and %s",
      hk_type_name(val1.type), hk_type_name(val2.type));
    return;
  }
  double data = fmod(hk_as_number(val1), hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --vm->stackTop;
}

static inline void do_negate(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_vm_runtime_error(vm, "type error: cannot apply `negate` to %s", hk_type_name(val.type));
    return;
  }
  double data = -hk_as_number(val);
  slots[0] = hk_number_value(data);
}

static inline void do_not(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop];
  HkValue val = slots[0];
  slots[0] = hk_is_falsey(val) ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  hk_value_release(val);
}

static inline void do_bitwise_not(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_vm_runtime_error(vm, "type error: cannot apply `bitwise not` to %s", hk_type_name(val.type));
    return;
  }
  double data = (double) (~((int64_t) hk_as_number(val)));
  slots[0] = hk_number_value(data);
}

static inline void do_increment(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_vm_runtime_error(vm, "type error: cannot increment value of type %s",
      hk_type_name(val.type));
    return;
  }
  ++slots[0].as.number;
}

static inline void do_decrement(HkVM *vm)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_vm_runtime_error(vm, "type error: cannot decrement value of type %s",
      hk_type_name(val.type));
    return;
  }
  --slots[0].as.number;
}

static inline void do_call(HkVM *vm, int numArgs)
{
  HkValue *slots = &vm->stackSlots[vm->stackTop - numArgs];
  HkValue val = slots[0];
  if (!hk_is_callable(val))
  {
    hk_vm_runtime_error(vm, "type error: cannot call value of type %s",
      hk_type_name(val.type));
    discard_frame(vm, slots);
    return;
  }
  if (hk_is_native(val))
  {
    HkNative *native = hk_as_native(val);
    adjust_call_args(vm, native->arity, numArgs);
    if (!hk_vm_is_ok(vm))
    {
      discard_frame(vm, slots);
      return;
    }
    native->call(vm, slots);
    HkSateStatus status = vm->status;
    if (status != HK_VM_STATUS_OK)
    {
      if (hk_vm_is_no_trace(vm))
      {
        if (hk_vm_is_error(vm))
          vm->flags = HK_VM_FLAG_NONE;
      }
      else
        print_trace(native->name, NULL, 0);
      if (status == HK_VM_STATUS_ERROR)
      {
        discard_frame(vm, slots);
        return;
      }
      hk_assert(status == HK_VM_STATUS_EXIT, "status should be exit");
    }
    hk_native_release(native);
    move_result(vm, slots);
    return;
  }
  HkClosure *cl = hk_as_closure(val);
  HkFunction *fn = cl->fn;
  adjust_call_args(vm, fn->arity, numArgs);
  if (!hk_vm_is_ok(vm))
  {
    discard_frame(vm, slots);
    return;
  }
  int line;
  call_function(vm, slots, cl, &line);
  HkSateStatus status = vm->status;
  if (status != HK_VM_STATUS_OK)
  {
    if (hk_vm_is_no_trace(vm))
    {
      if (hk_vm_is_error(vm))
        vm->flags = HK_VM_FLAG_NONE;
    }
    else
      print_trace(fn->name, fn->file, line);
    if (status == HK_VM_STATUS_ERROR)
    {
      discard_frame(vm, slots);
      return;
    }
    hk_assert(status == HK_VM_STATUS_EXIT, "status should be exit");
  }
  hk_closure_release(cl);
  move_result(vm, slots);
}

static inline void adjust_call_args(HkVM *vm, int arity, int numArgs)
{
  if (numArgs > arity)
  {
    do
    {
      pop(vm);
      --numArgs;
    }
    while (numArgs > arity);
    return;
  }
  while (numArgs < arity)
  {
    push(vm, HK_NIL_VALUE);
    hk_return_if_not_ok(vm);
    ++numArgs;
  }
}

static inline void print_trace(HkString *name, HkString *file, int line)
{
  char *nameChars = name ? name->chars : "<anonymous>";
  if (file)
  {
    fprintf(stderr, "  at %s() in %.*s:%d\n", nameChars, file->length, file->chars, line);
    return;
  }
  fprintf(stderr, "  at %s() in <native>\n", nameChars);
}

static inline void call_function(HkVM *vm, HkValue *locals, HkClosure *cl, int *line)
{
  HkValue *slots = vm->stackSlots;
  HkFunction *fn = cl->fn;
  HkValue *nonlocals = cl->nonlocals;
  HkChunk *chunk = &fn->chunk;
  uint8_t *code = chunk->code;
  HkValue *consts = chunk->consts->elements;
  HkFunction **functions = fn->functions;
  uint8_t *pc = code;
  for (;;)
  {
    HkOpCode op = (HkOpCode) read_byte(&pc);
    switch (op)
    {
    case HK_OP_NIL:
      push(vm, HK_NIL_VALUE);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_FALSE:
      push(vm, HK_FALSE_VALUE);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_TRUE:
      push(vm, HK_TRUE_VALUE);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_INT:
      push(vm, hk_number_value(read_word(&pc)));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_CONSTANT:
      {
        HkValue val = consts[read_byte(&pc)];
        push(vm, val);
        if (!hk_vm_is_ok(vm))
          goto end;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_RANGE:
      do_range(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_ARRAY:
      do_array(vm, read_byte(&pc));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_STRUCT:
      do_struct(vm, read_byte(&pc));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_INSTANCE:
      do_instance(vm, read_byte(&pc));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_CONSTRUCT:
      do_construct(vm, read_byte(&pc));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_ITERATOR:
      do_iterator(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_CLOSURE:
      do_closure(vm, functions[read_byte(&pc)]);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_UNPACK_ARRAY:
      do_unpack_array(vm, read_byte(&pc));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_UNPACK_STRUCT:
      do_unpack_struct(vm, read_byte(&pc));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_POP:
      hk_value_release(slots[vm->stackTop--]);
      break;
    case HK_OP_GLOBAL:
      {
        HkValue val = slots[read_byte(&pc)];
        push(vm, val);
        if (!hk_vm_is_ok(vm))
          goto end;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_NONLOCAL:
      {
        HkValue val = nonlocals[read_byte(&pc)];
        push(vm, val);
        if (!hk_vm_is_ok(vm))
          goto end;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_GET_LOCAL:
      {
        HkValue val = locals[read_byte(&pc)];
        push(vm, val);
        if (!hk_vm_is_ok(vm))
          goto end;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_SET_LOCAL:
      {
        int index = read_byte(&pc);
        HkValue val = slots[vm->stackTop];
        --vm->stackTop;
        hk_value_release(locals[index]);
        locals[index] = val;
      }
      break;
    case HK_OP_ADD_ELEMENT:
      do_add_element(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_GET_ELEMENT:
      do_get_element(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_FETCH_ELEMENT:
      do_fetch_element(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_SET_ELEMENT:
      do_set_element(vm);
      break;
    case HK_OP_PUT_ELEMENT:
      do_put_element(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_DELETE_ELEMENT:
      do_delete_element(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_INPLACE_ADD_ELEMENT:
      do_inplace_add_element(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_INPLACE_PUT_ELEMENT:
      do_inplace_put_element(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_INPLACE_DELETE_ELEMENT:
      do_inplace_delete_element(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_GET_FIELD:
      do_get_field(vm, hk_as_string(consts[read_byte(&pc)]));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_FETCH_FIELD:
      do_fetch_field(vm, hk_as_string(consts[read_byte(&pc)]));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_SET_FIELD:
      do_set_field(vm);
      break;
    case HK_OP_PUT_FIELD:
      do_put_field(vm, hk_as_string(consts[read_byte(&pc)]));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_INPLACE_PUT_FIELD:
      do_inplace_put_field(vm, hk_as_string(consts[read_byte(&pc)]));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_CURRENT:
      do_current(vm);
      break;
    case HK_OP_JUMP:
      pc = &code[read_word(&pc)];
      break;
    case HK_OP_JUMP_IF_FALSE:
      {
        int offset = read_word(&pc);
        HkValue val = slots[vm->stackTop];
        if (hk_is_falsey(val))
          pc = &code[offset];
        hk_value_release(val);
        --vm->stackTop;
      }
      break;
    case HK_OP_JUMP_IF_TRUE:
      {
        int offset = read_word(&pc);
        HkValue val = slots[vm->stackTop];
        if (hk_is_truthy(val))
          pc = &code[offset];
        hk_value_release(val);
        --vm->stackTop;
      }
      break;
    case HK_OP_JUMP_IF_TRUE_OR_POP:
      {
        int offset = read_word(&pc);
        HkValue val = slots[vm->stackTop];
        if (hk_is_truthy(val))
        {
          pc = &code[offset];
          break;
        }
        hk_value_release(val);
        --vm->stackTop;
      }
      break;
    case HK_OP_JUMP_IF_FALSE_OR_POP:
      {
        int offset = read_word(&pc);
        HkValue val = slots[vm->stackTop];
        if (hk_is_falsey(val))
        {
          pc = &code[offset];
          break;
        }
        hk_value_release(val);
        --vm->stackTop;
      }
      break;
    case HK_OP_JUMP_IF_NOT_EQUAL:
      {
        int offset = read_word(&pc);
        HkValue val1 = slots[vm->stackTop - 1];
        HkValue val2 = slots[vm->stackTop];
        if (hk_value_equal(val1, val2))
        {
          hk_value_release(val1);
          hk_value_release(val2);
          vm->stackTop -= 2;
          break;
        }
        pc = &code[offset];
        hk_value_release(val2);
        --vm->stackTop;
      }
      break;
    case HK_OP_JUMP_IF_NOT_VALID:
      {
        int offset = read_word(&pc);
        HkValue val = slots[vm->stackTop];
        HkIterator *it = hk_as_iterator(val);
        if (!hk_iterator_is_valid(it))
          pc = &code[offset];
      }
      break;
    case HK_OP_NEXT:
      do_next(vm);
      break;
    case HK_OP_EQUAL:
      do_equal(vm);
      break;
    case HK_OP_GREATER:
      do_greater(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_LESS:
      do_less(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_NOT_EQUAL:
      do_not_equal(vm);
      break;
    case HK_OP_NOT_GREATER:
      do_not_greater(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_NOT_LESS:
      do_not_less(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_BITWISE_OR:
      do_bitwise_or(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_BITWISE_XOR:
      do_bitwise_xor(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_BITWISE_AND:
      do_bitwise_and(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_LEFT_SHIFT:
      do_left_shift(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_RIGHT_SHIFT:
      do_right_shift(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_ADD:
      do_add(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_SUBTRACT:
      do_subtract(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_MULTIPLY:
      do_multiply(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_DIVIDE:
      do_divide(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_QUOTIENT:
      do_quotient(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_REMAINDER:
      do_remainder(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_NEGATE:
      do_negate(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_NOT:
      do_not(vm);
      break;
    case HK_OP_BITWISE_NOT:
      do_bitwise_not(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_INCREMENT:
      do_increment(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_DECREMENT:
      do_decrement(vm);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_CALL:
      do_call(vm, read_byte(&pc));
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_LOAD_MODULE:
      module_load(vm, fn->file);
      if (!hk_vm_is_ok(vm))
        goto end;
      break;
    case HK_OP_RETURN:
      return;
    case HK_OP_RETURN_NIL:
      push(vm, HK_NIL_VALUE);
      if (!hk_vm_is_ok(vm))
        goto end;
      return;
    }
  }
end:
  *line = hk_chunk_get_line(chunk, (int) (pc - code));
}

static inline void discard_frame(HkVM *vm, HkValue *slots)
{
  while (&vm->stackSlots[vm->stackTop] >= slots)
    hk_value_release(vm->stackSlots[vm->stackTop--]);
}

static inline void move_result(HkVM *vm, HkValue *slots)
{
  slots[0] = vm->stackSlots[vm->stackTop];
  --vm->stackTop;
  while (&vm->stackSlots[vm->stackTop] > slots)
    hk_value_release(vm->stackSlots[vm->stackTop--]);
}

void hk_vm_init(HkVM *vm, int minCapacity)
{
  int capacity = minCapacity < HK_STACK_MIN_CAPACITY ? HK_STACK_MIN_CAPACITY : minCapacity;
  capacity = hk_power_of_two_ceil(capacity);
  vm->stackEnd = capacity - 1;
  vm->stackTop = -1;
  vm->stackSlots = (HkValue *) hk_allocate(sizeof(*vm->stackSlots) * capacity);
  vm->flags = HK_VM_FLAG_NONE;
  vm->status = HK_VM_STATUS_OK;
  load_globals(vm);
  hk_assert(hk_vm_is_ok(vm), "vm should be ok");
  module_cache_init();
}

void hk_vm_deinit(HkVM *vm)
{
  module_cache_deinit();
  hk_assert(vm->stackTop == num_globals() - 1, "stack must contain the globals");
  while (vm->stackTop > -1)
    hk_value_release(vm->stackSlots[vm->stackTop--]);
  free(vm->stackSlots);
}

void hk_vm_runtime_error(HkVM *vm, const char *fmt, ...)
{
  vm->status = HK_VM_STATUS_ERROR;
  fprintf(stderr, "runtime error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

void hk_vm_check_argument_type(HkVM *vm, HkValue *args, int index, HkType type)
{
  HkType valType = args[index].type;
  if (valType != type)
    hk_vm_runtime_error(vm, "type error: argument #%d must be of the type %s, %s given", index,
      hk_type_name(type), hk_type_name(valType));
}

void hk_vm_check_argument_types(HkVM *vm, HkValue *args, int index, int numTypes, HkType types[])
{
  HkType valType = args[index].type;
  bool match = false;
  for (int i = 0; i < numTypes; ++i)
  {
    match = valType == types[i];
    if (match) break;
  }
  if (!match)
    type_error(vm, index, numTypes, types, valType);
}

void hk_vm_check_argument_bool(HkVM *vm, HkValue *args, int index)
{
  hk_vm_check_argument_type(vm, args, index, HK_TYPE_BOOL);
}

void hk_vm_check_argument_number(HkVM *vm, HkValue *args, int index)
{
  hk_vm_check_argument_type(vm, args, index, HK_TYPE_NUMBER);
}

void hk_vm_check_argument_int(HkVM *vm, HkValue *args, int index)
{
  HkValue val = args[index];
  if (!hk_is_int(val))
    hk_vm_runtime_error(vm, "type error: argument #%d must be of the type int, %s given",
      index, hk_type_name(val.type));
}

void hk_vm_check_argument_string(HkVM *vm, HkValue *args, int index)
{
  hk_vm_check_argument_type(vm, args, index, HK_TYPE_STRING);
}

void hk_vm_check_argument_range(HkVM *vm, HkValue *args, int index)
{
  hk_vm_check_argument_type(vm, args, index, HK_TYPE_RANGE);
}

void hk_vm_check_argument_array(HkVM *vm, HkValue *args, int index)
{
  hk_vm_check_argument_type(vm, args, index, HK_TYPE_ARRAY);
}

void hk_vm_check_argument_struct(HkVM *vm, HkValue *args, int index)
{
  hk_vm_check_argument_type(vm, args, index, HK_TYPE_STRUCT);
}

void hk_vm_check_argument_instance(HkVM *vm, HkValue *args, int index)
{
  hk_vm_check_argument_type(vm, args, index, HK_TYPE_INSTANCE);
}

void hk_vm_check_argument_iterator(HkVM *vm, HkValue *args, int index)
{
  hk_vm_check_argument_type(vm, args, index, HK_TYPE_ITERATOR);
}

void hk_vm_check_argument_callable(HkVM *vm, HkValue *args, int index)
{
  hk_vm_check_argument_type(vm, args, index, HK_TYPE_CALLABLE);
}

void hk_vm_check_argument_userdata(HkVM *vm, HkValue *args, int index)
{
  hk_vm_check_argument_type(vm, args, index, HK_TYPE_USERDATA);
}

void hk_vm_push(HkVM *vm, HkValue val)
{
  push(vm, val);
  hk_return_if_not_ok(vm);
  hk_value_incr_ref(val);
}

void hk_vm_push_nil(HkVM *vm)
{
  push(vm, HK_NIL_VALUE);
}

void hk_vm_push_bool(HkVM *vm, bool data)
{
  push(vm, data ? HK_TRUE_VALUE : HK_FALSE_VALUE);
}

void hk_vm_push_number(HkVM *vm, double data)
{
  push(vm, hk_number_value(data));
}

void hk_vm_push_string(HkVM *vm, HkString *str)
{
  push(vm, hk_string_value(str));
  hk_return_if_not_ok(vm);
  hk_incr_ref(str);
}

void hk_vm_push_string_from_chars(HkVM *vm, int length, const char *chars)
{
  HkString *str = hk_string_from_chars(length, chars);
  hk_vm_push_string(vm, str);
  if (!hk_vm_is_ok(vm))
    hk_string_free(str);
}

void hk_vm_push_string_from_stream(HkVM *vm, FILE *stream, const char delim)
{
  HkString *str = hk_string_from_stream(stream, delim);
  hk_vm_push_string(vm, str);
  if (!hk_vm_is_ok(vm))
    hk_string_free(str);
}

void hk_vm_push_range(HkVM *vm, HkRange *range)
{
  push(vm, hk_range_value(range));
  hk_return_if_not_ok(vm);
  hk_incr_ref(range);
}

void hk_vm_push_array(HkVM *vm, HkArray *arr)
{
  push(vm, hk_array_value(arr));
  hk_return_if_not_ok(vm);
  hk_incr_ref(arr);
}

void hk_vm_push_struct(HkVM *vm, HkStruct *ztruct)
{
  push(vm, hk_struct_value(ztruct));
  hk_return_if_not_ok(vm);
  hk_incr_ref(ztruct);
}

void hk_vm_push_instance(HkVM *vm, HkInstance *inst)
{
  push(vm, hk_instance_value(inst));
  hk_return_if_not_ok(vm);
  hk_incr_ref(inst);
}

void hk_vm_push_iterator(HkVM *vm, HkIterator *it)
{
  push(vm, hk_iterator_value(it));
  hk_return_if_not_ok(vm);
  hk_incr_ref(it);
}

void hk_vm_push_closure(HkVM *vm, HkClosure *cl)
{
  push(vm, hk_closure_value(cl));
  hk_return_if_not_ok(vm);
  hk_incr_ref(cl);
}

void hk_vm_push_native(HkVM *vm, HkNative *native)
{
  push(vm, hk_native_value(native));
  hk_return_if_not_ok(vm);
  hk_incr_ref(native);
}

void hk_vm_push_new_native(HkVM *vm, const char *name, int arity,
  HkCallFn call)
{
  HkNative *native = hk_native_new(hk_string_from_chars(-1, name), arity, call);
  hk_vm_push_native(vm, native);
  if (!hk_vm_is_ok(vm))
    hk_native_free(native);
}

void hk_vm_push_userdata(HkVM *vm, HkUserdata *udata)
{
  push(vm, hk_userdata_value(udata));
  hk_return_if_not_ok(vm);
  hk_incr_ref(udata);
}

void hk_vm_array(HkVM *vm, int length)
{
  do_array(vm, length);
}

void hk_vm_struct(HkVM *vm, int length)
{
  do_struct(vm, length);
}

void hk_vm_instance(HkVM *vm, int numArgs)
{
  do_instance(vm, numArgs);
}

void hk_vm_construct(HkVM *vm, int length)
{
  do_construct(vm, length);
}

void hk_vm_pop(HkVM *vm)
{
  pop(vm);
}

void hk_vm_call(HkVM *vm, int numArgs)
{
  do_call(vm, numArgs);
}

void hk_vm_compare(HkVM *vm, HkValue val1, HkValue val2, int *result)
{
  if (!hk_is_comparable(val1))
  {
    hk_vm_runtime_error(vm, "type error: value of type %s is not comparable", hk_type_name(val1.type));
    return;
  }
  if (val1.type != val2.type)
  {
    hk_vm_runtime_error(vm, "type error: cannot compare %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  hk_assert(hk_value_compare(val1, val2, result), "hk_value_compare failed");
}
