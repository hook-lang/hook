//
// Hook Programming Language
// hook_vm.c
//

#include "hook_vm.h"
#include <stdlib.h>
#include <math.h>
#include "hook_builtin.h"
#include "hook_struct.h"
#include "hook_module.h"
#include "hook_utils.h"
#include "hook_memory.h"
#include "hook_status.h"
#include "hook_error.h"

static inline void type_error(int32_t index, int32_t num_types, int32_t types[], int32_t val_type);
static inline int32_t push(hk_vm_t *vm, hk_value_t val);
static inline void pop(hk_vm_t *vm);
static inline int32_t read_byte(uint8_t **pc);
static inline int32_t read_word(uint8_t **pc);
static inline int32_t do_range(hk_vm_t *vm);
static inline int32_t do_array(hk_vm_t *vm, int32_t length);
static inline int32_t do_struct(hk_vm_t *vm, int32_t length);
static inline int32_t do_instance(hk_vm_t *vm, int32_t num_args);
static inline int32_t adjust_instance_args(hk_vm_t *vm, int32_t length, int32_t num_args);
static inline int32_t do_construct(hk_vm_t *vm, int32_t length);
static inline int32_t do_closure(hk_vm_t *vm, hk_function_t *fn);
static inline int32_t do_unpack(hk_vm_t *vm, int32_t n);
static inline int32_t do_destruct(hk_vm_t *vm, int32_t n);
static inline int32_t do_add_element(hk_vm_t *vm);
static inline int32_t do_get_element(hk_vm_t *vm);
static inline void slice_string(hk_vm_t *vm, hk_value_t *slot, hk_string_t *str, hk_range_t *range);
static inline void slice_array(hk_vm_t *vm, hk_value_t *slot, hk_array_t *arr, hk_range_t *range);
static inline int32_t do_fetch_element(hk_vm_t *vm);
static inline void do_set_element(hk_vm_t *vm);
static inline int32_t do_put_element(hk_vm_t *vm);
static inline int32_t do_delete_element(hk_vm_t *vm);
static inline int32_t do_inplace_add_element(hk_vm_t *vm);
static inline int32_t do_inplace_put_element(hk_vm_t *vm);
static inline int32_t do_inplace_delete_element(hk_vm_t *vm);
static inline int32_t do_get_field(hk_vm_t *vm, hk_string_t *name);
static inline int32_t do_fetch_field(hk_vm_t *vm, hk_string_t *name);
static inline void do_set_field(hk_vm_t *vm);
static inline int32_t do_put_field(hk_vm_t *vm, hk_string_t *name);
static inline int32_t do_inplace_put_field(hk_vm_t *vm, hk_string_t *name);
static inline void do_equal(hk_vm_t *vm);
static inline int32_t do_greater(hk_vm_t *vm);
static inline int32_t do_less(hk_vm_t *vm);
static inline void do_not_equal(hk_vm_t *vm);
static inline int32_t do_not_greater(hk_vm_t *vm);
static inline int32_t do_not_less(hk_vm_t *vm);
static inline int32_t do_bitwise_or(hk_vm_t *vm);
static inline int32_t do_bitwise_xor(hk_vm_t *vm);
static inline int32_t do_bitwise_and(hk_vm_t *vm);
static inline int32_t do_left_shift(hk_vm_t *vm);
static inline int32_t do_right_shift(hk_vm_t *vm);
static inline int32_t do_add(hk_vm_t *vm);
static inline int32_t concat_strings(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2);
static inline int32_t concat_arrays(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2);
static inline int32_t do_subtract(hk_vm_t *vm);
static inline int32_t diff_arrays(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2);
static inline int32_t do_multiply(hk_vm_t *vm);
static inline int32_t do_divide(hk_vm_t *vm);
static inline int32_t do_quotient(hk_vm_t *vm);
static inline int32_t do_remainder(hk_vm_t *vm);
static inline int32_t do_negate(hk_vm_t *vm);
static inline void do_not(hk_vm_t *vm);
static inline int32_t do_bitwise_not(hk_vm_t *vm);
static inline int32_t do_incr(hk_vm_t *vm);
static inline int32_t do_decr(hk_vm_t *vm);
static inline int32_t do_call(hk_vm_t *vm, int32_t num_args);
static inline int32_t adjust_call_args(hk_vm_t *vm, int32_t arity, int32_t num_args);
static inline void print_trace(hk_string_t *name, hk_string_t *file, int32_t line);
static inline int32_t call_function(hk_vm_t *vm, hk_value_t *locals, hk_closure_t *cl, int32_t *line);
static inline void discard_frame(hk_vm_t *vm, hk_value_t *slots);
static inline void move_result(hk_vm_t *vm, hk_value_t *slots);

static inline void type_error(int32_t index, int32_t num_types, int32_t types[], int32_t val_type)
{
  hk_assert(num_types > 0, "num_types must be greater than 0");
  fprintf(stderr, "runtime error: type error: argument #%d must be of the type %s",
    index, hk_type_name(types[0]));
  for (int32_t i = 1; i < num_types; ++i)
    fprintf(stderr, "|%s", hk_type_name(types[i]));
  fprintf(stderr, ", %s given\n", hk_type_name(val_type));
}

static inline int32_t push(hk_vm_t *vm, hk_value_t val)
{
  if (vm->stack_top == vm->stack_end)
  {
    hk_runtime_error("stack overflow");
    return HK_STATUS_ERROR;
  }
  ++vm->stack_top;
  vm->stack[vm->stack_top] = val;
  return HK_STATUS_OK;
}

static inline void pop(hk_vm_t *vm)
{
  hk_assert(vm->stack_top > -1, "stack underflow");
  hk_value_t val = vm->stack[vm->stack_top];
  --vm->stack_top;
  hk_value_release(val);
}

static inline int32_t read_byte(uint8_t **pc)
{
  int32_t byte = **pc;
  ++(*pc);
  return byte;
}

static inline int32_t read_word(uint8_t **pc)
{
  int32_t word = *((uint16_t *) *pc);
  *pc += 2;
  return word;
}

static inline int32_t do_range(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_float(val1) || !hk_is_float(val2))
  {
    hk_runtime_error("type error: range must be of type float");
    return HK_STATUS_ERROR;
  }
  hk_range_t *range = hk_range_new(val1.as_float, val2.as_float);
  hk_incr_ref(range);
  slots[0] = hk_range_value(range);
  --vm->stack_top;
  return HK_STATUS_OK;
}

static inline int32_t do_array(hk_vm_t *vm, int32_t length)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - length + 1];
  hk_array_t *arr = hk_array_new_with_capacity(length);
  arr->length = length;
  for (int32_t i = 0; i < length; ++i)
    arr->elements[i] = slots[i];
  vm->stack_top -= length;
  if (push(vm, hk_array_value(arr)) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  hk_incr_ref(arr);
  return HK_STATUS_OK;
}

static inline int32_t do_struct(hk_vm_t *vm, int32_t length)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - length];
  hk_value_t val = slots[0];
  hk_string_t *struct_name = hk_is_nil(val) ? NULL : hk_as_string(val);
  hk_struct_t *ztruct = hk_struct_new(struct_name);
  for (int32_t i = 1; i <= length; ++i)
  {
    hk_string_t *field_name = hk_as_string(slots[i]);
    if (!hk_struct_define_field(ztruct, field_name))
    {
      hk_runtime_error("field %.*s is already defined", field_name->length,
        field_name->chars);
      hk_struct_free(ztruct);
      return HK_STATUS_ERROR;
    }
  }
  for (int32_t i = 1; i <= length; ++i)
    hk_decr_ref(hk_as_object(slots[i]));
  vm->stack_top -= length;
  hk_incr_ref(ztruct);
  slots[0] = hk_struct_value(ztruct);
  if (struct_name)
    hk_decr_ref(struct_name);
  return HK_STATUS_OK;
}

static inline int32_t do_instance(hk_vm_t *vm, int32_t num_args)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - num_args];
  hk_value_t val = slots[0];
  if (!hk_is_struct(val))
  {
    hk_runtime_error("type error: cannot use %s as a struct", hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  hk_struct_t *ztruct = hk_as_struct(val);
  int32_t length = ztruct->length;
  adjust_instance_args(vm, length, num_args);
  hk_instance_t *inst = hk_instance_new(ztruct);
  for (int32_t i = 0; i < length; ++i)
    inst->values[i] = slots[i + 1];
  vm->stack_top -= length;
  hk_incr_ref(inst);
  slots[0] = hk_instance_value(inst);
  hk_struct_release(ztruct);
  return HK_STATUS_OK;
}

static inline int32_t adjust_instance_args(hk_vm_t *vm, int32_t length, int32_t num_args)
{
  if (num_args > length)
  {
    do
    {
      pop(vm);
      --num_args;
    }
    while (num_args > length);
    return HK_STATUS_OK;
  }
  while (num_args < length)
  {
    if (push(vm, HK_NIL_VALUE) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    ++num_args;
  }
  return HK_STATUS_OK;
}

static inline int32_t do_construct(hk_vm_t *vm, int32_t length)
{
  int32_t n = length << 1;
  hk_value_t *slots = &vm->stack[vm->stack_top - n];
  hk_value_t val = slots[0];
  hk_string_t *struct_name = hk_is_nil(val) ? NULL : hk_as_string(val);
  hk_struct_t *ztruct = hk_struct_new(struct_name);
  for (int32_t i = 1; i <= n; i += 2)
  {
    hk_string_t *field_name = hk_as_string(slots[i]);
    if (hk_struct_define_field(ztruct, field_name))
      continue;
    hk_runtime_error("field %.*s is already defined", field_name->length,
      field_name->chars);
    hk_struct_free(ztruct);
    return HK_STATUS_ERROR;
  }
  for (int32_t i = 1; i <= n; i += 2)
    hk_decr_ref(hk_as_object(slots[i]));
  hk_instance_t *inst = hk_instance_new(ztruct);
  for (int32_t i = 2, j = 0; i <= n + 1; i += 2, ++j)
    inst->values[j] = slots[i];
  vm->stack_top -= n;
  hk_incr_ref(inst);
  slots[0] = hk_instance_value(inst);
  if (struct_name)
    hk_decr_ref(struct_name);
  return HK_STATUS_OK;
}

static inline int32_t do_closure(hk_vm_t *vm, hk_function_t *fn)
{
  int32_t num_nonlocals = fn->num_nonlocals;
  hk_value_t *slots = &vm->stack[vm->stack_top - num_nonlocals + 1];
  hk_closure_t *cl = hk_closure_new(fn);
  for (int32_t i = 0; i < num_nonlocals; ++i)
    cl->nonlocals[i] = slots[i];
  vm->stack_top -= num_nonlocals;
  if (push(vm, hk_closure_value(cl)) == HK_STATUS_ERROR)
  {
    hk_closure_free(cl);
    return HK_STATUS_ERROR;
  }
  hk_incr_ref(cl);
  return HK_STATUS_OK;
}

static inline int32_t do_unpack(hk_vm_t *vm, int32_t n)
{
  hk_value_t val = vm->stack[vm->stack_top];
  if (!hk_is_array(val))
  {
    hk_runtime_error("type error: cannot unpack value of type %s",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val);
  --vm->stack_top;
  int32_t status = HK_STATUS_OK;
  for (int32_t i = 0; i < n && i < arr->length; ++i)
  {
    hk_value_t elem = hk_array_get_element(arr, i);
    if ((status = push(vm, elem)) == HK_STATUS_ERROR)
      goto end;
    hk_value_incr_ref(elem);
  }
  for (int32_t i = arr->length; i < n; ++i)
    if ((status = push(vm, HK_NIL_VALUE)) == HK_STATUS_ERROR)
      break;
end:
  hk_array_release(arr);
  return status;
}

static inline int32_t do_destruct(hk_vm_t *vm, int32_t n)
{
  hk_value_t val = vm->stack[vm->stack_top];
  if (!hk_is_instance(val))
  {
    hk_runtime_error("type error: cannot destructure value of type %s",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  hk_instance_t *inst = hk_as_instance(val);
  hk_struct_t *ztruct = inst->ztruct;
  hk_value_t *slots = &vm->stack[vm->stack_top - n];
  for (int32_t i = 0; i < n; ++i)
  {
    hk_string_t *name = hk_as_string(slots[i]);
    int32_t index = hk_struct_index_of(ztruct, name);
    hk_value_t value = index == -1 ? HK_NIL_VALUE :
      hk_instance_get_field(inst, index);
    hk_value_incr_ref(value);
    hk_decr_ref(name);
    slots[i] = value;
  }
  --vm->stack_top;
  hk_instance_release(inst);
  return HK_STATUS_OK;
}

static inline int32_t do_add_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  hk_array_t *result = hk_array_add_element(arr, val2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stack_top;
  hk_array_release(arr);  
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline int32_t do_get_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (hk_is_string(val1))
  {
    hk_string_t *str = hk_as_string(val1);
    if (hk_is_int(val2))
    {
      int64_t index = (int64_t) val2.as_float;
      if (index < 0 || index >= str->length)
      {
        hk_runtime_error("range error: index %d is out of bounds for string of length %d",
          index, str->length);
        return HK_STATUS_ERROR;
      }
      hk_value_t result = hk_string_value(hk_string_from_chars(1, &str->chars[(int32_t) index]));
      hk_value_incr_ref(result);
      slots[0] = result;
      --vm->stack_top;
      hk_string_release(str);
      return HK_STATUS_OK;
    }
    if (!hk_is_range(val2))
    {
      hk_runtime_error("type error: string cannot be indexed by %s", hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    slice_string(vm, slots, str, hk_as_range(val2));
    return HK_STATUS_OK;
  }
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: %s cannot be indexed", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  if (hk_is_int(val2))
  {
    int64_t index = (int64_t) val2.as_float;
    if (index < 0 || index >= arr->length)
    {
      hk_runtime_error("range error: index %d is out of bounds for array of length %d",
        index, arr->length);
      return HK_STATUS_ERROR;
    }
    hk_value_t result = hk_array_get_element(arr, (int32_t) index);
    hk_value_incr_ref(result);
    slots[0] = result;
    --vm->stack_top;
    hk_array_release(arr);
    return HK_STATUS_OK;
  }
  if (!hk_is_range(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  slice_array(vm, slots, arr, hk_as_range(val2));
  return HK_STATUS_OK;
}

static inline void slice_string(hk_vm_t *vm, hk_value_t *slot, hk_string_t *str, hk_range_t *range)
{
  int32_t str_end = str->length - 1;
  int64_t start = range->start;
  int64_t end = range->end;
  hk_string_t *result;
  if (start > end || start > str_end || end < 0)
  {
    result = hk_string_new();
    goto end;
  }
  if (start <= 0 && end >= str_end)
  {
    --vm->stack_top;
    hk_range_release(range);
    return;
  }
  int32_t length = end - start + 1;
  result = hk_string_from_chars(length, &str->chars[start]);
end:
  hk_incr_ref(result);
  *slot = hk_string_value(result);
  --vm->stack_top;
  hk_string_release(str);
  hk_range_release(range);
}

static inline void slice_array(hk_vm_t *vm, hk_value_t *slot, hk_array_t *arr, hk_range_t *range)
{
  int32_t arr_end = arr->length - 1;
  int64_t start = range->start;
  int64_t end = range->end;
  hk_array_t *result;
  if (start > end || start > arr_end || end < 0)
  {
    result = hk_array_new();
    goto end;
  }
  if (start <= 0 && end >= arr_end)
  {
    --vm->stack_top;
    hk_range_release(range);
    return;
  }
  int32_t length = end - start + 1;
  result = hk_array_new_with_capacity(length);
  result->length = length;
  for (int32_t i = start, j = 0; i <= end ; ++i, ++j)
  {
    hk_value_t elem = hk_array_get_element(arr, i);
    hk_value_incr_ref(elem);
    result->elements[j] = elem;
  }
end:
  hk_incr_ref(result);
  *slot = hk_array_value(result);
  --vm->stack_top;
  hk_array_release(arr);
  hk_range_release(range);
}

static inline int32_t do_fetch_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  if (!hk_is_int(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  int64_t index = (int64_t) val2.as_float;
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  hk_value_t elem = hk_array_get_element(arr, (int32_t) index);
  if (push(vm, elem) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_incr_ref(elem);
  return HK_STATUS_OK;
}

static inline void do_set_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 2];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  hk_value_t val3 = slots[2];
  hk_array_t *arr = hk_as_array(val1);
  int32_t index = (int32_t) val2.as_float;
  hk_array_t *result = hk_array_set_element(arr, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  vm->stack_top -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
}

static inline int32_t do_put_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 2];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  hk_value_t val3 = slots[2];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  if (!hk_is_int(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  int64_t index = (int64_t) val2.as_float;
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  hk_array_t *result = hk_array_set_element(arr, (int32_t) index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  vm->stack_top -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
  return HK_STATUS_OK;
}

static inline int32_t do_delete_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  if (!hk_is_int(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  int64_t index = (int64_t) val2.as_float;
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  hk_array_t *result = hk_array_delete_element(arr, (int32_t) index);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stack_top;
  hk_array_release(arr);
  return HK_STATUS_OK;
}

static inline int32_t do_inplace_add_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  if (arr->ref_count == 2)
  {
    hk_array_inplace_add_element(arr, val2);
    --vm->stack_top;
    hk_value_decr_ref(val2);
    return HK_STATUS_OK;
  }
  hk_array_t *result = hk_array_add_element(arr, val2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stack_top;
  hk_array_release(arr);
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline int32_t do_inplace_put_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 2];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  hk_value_t val3 = slots[2];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  if (!hk_is_int(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  int64_t index = (int64_t) val2.as_float;
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  if (arr->ref_count == 2)
  {
    hk_array_inplace_set_element(arr, (int32_t) index, val3);
    vm->stack_top -= 2;
    hk_value_decr_ref(val3);
    return HK_STATUS_OK;
  }
  hk_array_t *result = hk_array_set_element(arr, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  vm->stack_top -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
  return HK_STATUS_OK;
}

static inline int32_t do_inplace_delete_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  if (!hk_is_int(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  int64_t index = (int64_t) val2.as_float;
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  if (arr->ref_count == 2)
  {
    hk_array_inplace_delete_element(arr, (int32_t) index);
    --vm->stack_top;
    return HK_STATUS_OK;
  }
  hk_array_t *result = hk_array_delete_element(arr, index);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stack_top;
  hk_array_release(arr);
  return HK_STATUS_OK;
}

static inline int32_t do_get_field(hk_vm_t *vm, hk_string_t *name)
{
  hk_value_t *slots = &vm->stack[vm->stack_top];
  hk_value_t val = slots[0];
  if (!hk_is_instance(val))
  {
    hk_runtime_error("type error: cannot use %s as an instance",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  hk_instance_t *inst = hk_as_instance(val);
  int32_t index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  hk_value_t value = hk_instance_get_field(inst, index);
  hk_value_incr_ref(value);
  slots[0] = value;
  hk_instance_release(inst);
  return HK_STATUS_OK;
}

static inline int32_t do_fetch_field(hk_vm_t *vm, hk_string_t *name)
{
  hk_value_t *slots = &vm->stack[vm->stack_top];
  hk_value_t val = slots[0];
  if (!hk_is_instance(val))
  {
    hk_runtime_error("type error: cannot use %s as an instance", hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  hk_instance_t *inst = hk_as_instance(val);
  int32_t index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  if (push(vm, hk_float_value(index)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t value = hk_instance_get_field(inst, index);
  if (push(vm, value) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_incr_ref(value);
  return HK_STATUS_OK;
}

static inline void do_set_field(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 2];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  hk_value_t val3 = slots[2];
  hk_instance_t *inst = hk_as_instance(val1);
  int32_t index = (int32_t) val2.as_float;
  hk_instance_t *result = hk_instance_set_field(inst, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  vm->stack_top -= 2;
  hk_instance_release(inst);
  hk_value_decr_ref(val3);
}

static inline int32_t do_put_field(hk_vm_t *vm, hk_string_t *name)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_instance(val1))
  {
    hk_runtime_error("type error: cannot use %s as an instance", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  hk_instance_t *inst = hk_as_instance(val1);
  int32_t index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  hk_instance_t *result = hk_instance_set_field(inst, index, val2);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  --vm->stack_top;
  hk_instance_release(inst);
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline int32_t do_inplace_put_field(hk_vm_t *vm, hk_string_t *name)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_instance(val1))
  {
    hk_runtime_error("type error: cannot use %s as an instance", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  hk_instance_t *inst = hk_as_instance(val1);
  int32_t index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  if (inst->ref_count == 2)
  {
    hk_instance_inplace_set_field(inst, index, val2);
    --vm->stack_top;
    hk_value_decr_ref(val2);
    return HK_STATUS_OK;
  }
  hk_instance_t *result = hk_instance_set_field(inst, index, val2);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  --vm->stack_top;
  hk_instance_release(inst);
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline void do_equal(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  slots[0] = hk_value_equal(val1, val2) ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --vm->stack_top;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline int32_t do_greater(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  int32_t result;
  if (hk_value_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result > 0 ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --vm->stack_top;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline int32_t do_less(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  int32_t result;
  if (hk_value_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result < 0 ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --vm->stack_top;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline void do_not_equal(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  slots[0] = hk_value_equal(val1, val2) ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --vm->stack_top;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline int32_t do_not_greater(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  int32_t result;
  if (hk_value_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result > 0 ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --vm->stack_top;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline int32_t do_not_less(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  int32_t result;
  if (hk_value_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result < 0 ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --vm->stack_top;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline int32_t do_bitwise_or(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_float(val1) || !hk_is_float(val2))
  {
    hk_runtime_error("type error: cannot apply `bitwise or` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ((int64_t) val1.as_float) | ((int64_t) val2.as_float);
  slots[0] = hk_float_value(data);
  --vm->stack_top;
  return HK_STATUS_OK;
}

static inline int32_t do_bitwise_xor(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_float(val1) || !hk_is_float(val2))
  {
    hk_runtime_error("type error: cannot apply `bitwise xor` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ((int64_t) val1.as_float) ^ ((int64_t) val2.as_float);
  slots[0] = hk_float_value(data);
  --vm->stack_top;
  return HK_STATUS_OK;
}

static inline int32_t do_bitwise_and(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_float(val1) || !hk_is_float(val2))
  {
    hk_runtime_error("type error: cannot apply `bitwise and` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ((int64_t) val1.as_float) & ((int64_t) val2.as_float);
  slots[0] = hk_float_value(data);
  --vm->stack_top;
  return HK_STATUS_OK;
}

static inline int32_t do_left_shift(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_float(val1) || !hk_is_float(val2))
  {
    hk_runtime_error("type error: cannot apply `left shift` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ((int64_t) val1.as_float) << ((int64_t) val2.as_float);
  slots[0] = hk_float_value(data);
  --vm->stack_top;
  return HK_STATUS_OK;
}

static inline int32_t do_right_shift(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_float(val1) || !hk_is_float(val2))
  {
    hk_runtime_error("type error: cannot apply `right shift` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ((int64_t) val1.as_float) >> ((int64_t) val2.as_float);
  slots[0] = hk_float_value(data);
  --vm->stack_top;
  return HK_STATUS_OK;
}

static inline int32_t do_add(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (hk_is_float(val1))
  {
    if (!hk_is_float(val2))
    {
      hk_runtime_error("type error: cannot add %s to float", hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    double data = val1.as_float + val2.as_float;
    slots[0] = hk_float_value(data);
    --vm->stack_top;
    return HK_STATUS_OK;
  }
  if (hk_is_string(val1))
  {
    if (!hk_is_string(val2))
    {
      hk_runtime_error("type error: cannot concatenate string and %s",
        hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    return concat_strings(vm, slots, val1, val2);
  }
  if (hk_is_array(val1))
  {
    if (!hk_is_array(val2))
    {
      hk_runtime_error("type error: cannot concatenate array and %s",
        hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    return concat_arrays(vm, slots, val1, val2);
  }
  hk_runtime_error("type error: cannot add %s to %s", hk_type_name(val2.type),
    hk_type_name(val1.type));
  return HK_STATUS_ERROR;
}

static inline int32_t concat_strings(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2)
{
  hk_string_t *str1 = hk_as_string(val1);
  if (!str1->length)
  {
    slots[0] = val2;
    --vm->stack_top;
    hk_string_release(str1);
    return HK_STATUS_OK;
  }
  hk_string_t *str2 = hk_as_string(val2);
  if (!str2->length)
  {
    --vm->stack_top;
    hk_string_release(str2);
    return HK_STATUS_OK;
  }
  if (str1->ref_count == 1)
  {
    hk_string_inplace_concat(str1, str2);
    --vm->stack_top;
    hk_string_release(str2);
    return HK_STATUS_OK;
  }
  hk_string_t *result = hk_string_concat(str1, str2);
  hk_incr_ref(result);
  slots[0] = hk_string_value(result);
  --vm->stack_top;
  hk_string_release(str1);
  hk_string_release(str2);
  return HK_STATUS_OK;
}

static inline int32_t concat_arrays(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2)
{
  hk_array_t *arr1 = hk_as_array(val1);
  if (!arr1->length)
  {
    slots[0] = val2;
    --vm->stack_top;
    hk_array_release(arr1);
    return HK_STATUS_OK;
  }
  hk_array_t *arr2 = hk_as_array(val2);
  if (!arr2->length)
  {
    --vm->stack_top;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  if (arr1->ref_count == 1)
  {
    hk_array_inplace_concat(arr1, arr2);
    --vm->stack_top;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  hk_array_t *result = hk_array_concat(arr1, arr2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stack_top;
  hk_array_release(arr1);
  hk_array_release(arr2);
  return HK_STATUS_OK;
}

static inline int32_t do_subtract(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (hk_is_float(val1))
  {
    if (!hk_is_float(val2))
    {
      hk_runtime_error("type error: cannot subtract %s from float",
        hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    double data = val1.as_float - val2.as_float;
    slots[0] = hk_float_value(data);
    --vm->stack_top;
    return HK_STATUS_OK;
  }
  if (hk_is_array(val1))
  {
    if (!hk_is_array(val2))
    {
      hk_runtime_error("type error: cannot diff between array and %s",
        hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    return diff_arrays(vm, slots, val1, val2);
  }
  hk_runtime_error("type error: cannot subtract %s from %s", hk_type_name(val2.type),
    hk_type_name(val1.type));
  return HK_STATUS_ERROR;
}

static inline int32_t diff_arrays(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2)
{
  hk_array_t *arr1 = hk_as_array(val1);
  hk_array_t *arr2 = hk_as_array(val2);
  if (!arr1->length || !arr2->length)
  {
    --vm->stack_top;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  if (arr1->ref_count == 1)
  {
    hk_array_inplace_diff(arr1, arr2);
    --vm->stack_top;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  hk_array_t *result = hk_array_diff(arr1, arr2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->stack_top;
  hk_array_release(arr1);
  hk_array_release(arr2);
  return HK_STATUS_OK;
}

static inline int32_t do_multiply(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_float(val1) || !hk_is_float(val2))
  {
    hk_runtime_error("type error: cannot multiply %s to %s", hk_type_name(val2.type),
      hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  double data = val1.as_float * val2.as_float;
  slots[0] = hk_float_value(data);
  --vm->stack_top;
  return HK_STATUS_OK;
}

static inline int32_t do_divide(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_float(val1) || !hk_is_float(val2))
  {
    hk_runtime_error("type error: cannot divide %s by %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  double data = val1.as_float / val2.as_float;
  slots[0] = hk_float_value(data);
  --vm->stack_top;
  return HK_STATUS_OK;
}

static inline int32_t do_quotient(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_float(val1) || !hk_is_float(val2))
  {
    hk_runtime_error("type error: cannot apply `quotient` between %s and %s",
      hk_type_name(val1.type), hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  double data = floor(val1.as_float / val2.as_float);
  slots[0] = hk_float_value(data);
  --vm->stack_top;
  return HK_STATUS_OK;
}

static inline int32_t do_remainder(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_float(val1) || !hk_is_float(val2))
  {
    hk_runtime_error("type error: cannot apply `remainder` between %s and %s",
      hk_type_name(val1.type), hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  double data = fmod(val1.as_float, val2.as_float);
  slots[0] = hk_float_value(data);
  --vm->stack_top;
  return HK_STATUS_OK;
}

static inline int32_t do_negate(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top];
  hk_value_t val = slots[0];
  if (!hk_is_float(val))
  {
    hk_runtime_error("type error: cannot apply `negate` to %s", hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  double data = -val.as_float;
  slots[0] = hk_float_value(data);
  return HK_STATUS_OK;
}

static inline void do_not(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top];
  hk_value_t val = slots[0];
  slots[0] = hk_is_falsey(val) ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  hk_value_release(val);
}

static inline int32_t do_bitwise_not(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top];
  hk_value_t val = slots[0];
  if (!hk_is_float(val))
  {
    hk_runtime_error("type error: cannot apply `bitwise not` to %s", hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ~((int64_t) val.as_float);
  slots[0] = hk_float_value(data);
  return HK_STATUS_OK;
}

static inline int32_t do_incr(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top];
  hk_value_t val = slots[0];
  if (!hk_is_float(val))
  {
    hk_runtime_error("type error: cannot increment value of type %s",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  ++slots[0].as_float;
  return HK_STATUS_OK;
}

static inline int32_t do_decr(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->stack[vm->stack_top];
  hk_value_t val = slots[0];
  if (!hk_is_float(val))
  {
    hk_runtime_error("type error: cannot decrement value of type %s",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  --slots[0].as_float;
  return HK_STATUS_OK;
}

static inline int32_t do_call(hk_vm_t *vm, int32_t num_args)
{
  hk_value_t *slots = &vm->stack[vm->stack_top - num_args];
  hk_value_t val = slots[0];
  if (!hk_is_callable(val))
  {
    hk_runtime_error("type error: cannot call value of type %s",
      hk_type_name(val.type));
    discard_frame(vm, slots);
    return HK_STATUS_ERROR;
  }
  if (hk_is_native(val))
  {
    hk_native_t *native = hk_as_native(val);
    if (adjust_call_args(vm, native->arity, num_args) == HK_STATUS_ERROR)
    {
      discard_frame(vm, slots);
      return HK_STATUS_ERROR;
    }
    int32_t status;
    if ((status = native->call(vm, slots)) != HK_STATUS_OK)
    {
      if (status != HK_STATUS_NO_TRACE)
        print_trace(native->name, NULL, 0);
      discard_frame(vm, slots);
      return HK_STATUS_ERROR;
    }
    hk_native_release(native);
    move_result(vm, slots);
    return HK_STATUS_OK;
  }
  hk_closure_t *cl = hk_as_closure(val);
  hk_function_t *fn = cl->fn;
  if (adjust_call_args(vm, fn->arity, num_args) == HK_STATUS_ERROR)
  {
    discard_frame(vm, slots);
    return HK_STATUS_ERROR;
  }
  int32_t line;
  if (call_function(vm, slots, cl, &line) == HK_STATUS_ERROR)
  {
    print_trace(fn->name, fn->file, line);
    discard_frame(vm, slots);
    return HK_STATUS_ERROR;
  }
  hk_closure_release(cl);
  move_result(vm, slots);
  return HK_STATUS_OK;
}

static inline int32_t adjust_call_args(hk_vm_t *vm, int32_t arity,int32_t num_args)
{
  if (num_args >= arity)
    return HK_STATUS_OK;
  while (num_args < arity)
  {
    if (push(vm, HK_NIL_VALUE) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    ++num_args;
  }
  return HK_STATUS_OK;
}

static inline void print_trace(hk_string_t *name, hk_string_t *file, int32_t line)
{
  char *name_chars = name ? name->chars : "<anonymous>";
  if (file)
  {
    fprintf(stderr, "  at %s() in %.*s:%d\n", name_chars, file->length, file->chars, line);
    return;
  }
  fprintf(stderr, "  at %s() in <native>\n", name_chars);
}

static inline int32_t call_function(hk_vm_t *vm, hk_value_t *locals, hk_closure_t *cl, int32_t *line)
{
  hk_value_t *slots = vm->stack;
  hk_function_t *fn = cl->fn;
  hk_value_t *nonlocals = cl->nonlocals;
  hk_chunk_t *chunk = &fn->chunk;
  uint8_t *code = chunk->code;
  hk_value_t *consts = chunk->consts->elements;
  hk_function_t **functions = fn->functions;
  uint8_t *pc = code;
  for (;;)
  {
    int32_t op = read_byte(&pc);
    switch (op)
    {
    case HK_OP_NIL:
      if (push(vm, HK_NIL_VALUE) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_FALSE:
      if (push(vm, HK_FALSE_VALUE) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_TRUE:
      if (push(vm, HK_TRUE_VALUE) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INT:
      if (push(vm, hk_float_value(read_word(&pc))) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CONSTANT:
      {
        hk_value_t val = consts[read_byte(&pc)];
        if (push(vm, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_RANGE:
      if (do_range(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_ARRAY:
      if (do_array(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_STRUCT:
      if (do_struct(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INSTANCE:
      if (do_instance(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CONSTRUCT:
      if (do_construct(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CLOSURE:
      if (do_closure(vm, functions[read_byte(&pc)]) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_UNPACK:
      if (do_unpack(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_DESTRUCT:
      if (do_destruct(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_POP:
      hk_value_release(slots[vm->stack_top--]);
      break;
    case HK_OP_GLOBAL:
      {
        hk_value_t val = slots[read_byte(&pc)];
        if (push(vm, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_NONLOCAL:
      {
        hk_value_t val = nonlocals[read_byte(&pc)];
        if (push(vm, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_LOAD:
      {
        hk_value_t val = locals[read_byte(&pc)];
        if (push(vm, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_STORE:
      {
        int32_t index = read_byte(&pc);
        hk_value_t val = slots[vm->stack_top];
        --vm->stack_top;
        hk_value_release(locals[index]);
        locals[index] = val;
      }
      break;
    case HK_OP_ADD_ELEMENT:
      if (do_add_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_GET_ELEMENT:
      if (do_get_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_FETCH_ELEMENT:
      if (do_fetch_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_SET_ELEMENT:
      do_set_element(vm);
      break;
    case HK_OP_PUT_ELEMENT:
      if (do_put_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_DELETE_ELEMENT:
      if (do_delete_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_ADD_ELEMENT:
      if (do_inplace_add_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_PUT_ELEMENT:
      if (do_inplace_put_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_DELETE_ELEMENT:
      if (do_inplace_delete_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_GET_FIELD:
      if (do_get_field(vm, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_FETCH_FIELD:
      if (do_fetch_field(vm, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_SET_FIELD:
      do_set_field(vm);
      break;
    case HK_OP_PUT_FIELD:
      if (do_put_field(vm, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_PUT_FIELD:
      if (do_inplace_put_field(vm, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_JUMP:
      pc = &code[read_word(&pc)];
      break;
    case HK_OP_JUMP_IF_FALSE:
      {
        int32_t offset = read_word(&pc);
        hk_value_t val = slots[vm->stack_top];
        if (hk_is_falsey(val))
          pc = &code[offset];
        hk_value_release(val);
        --vm->stack_top;
      }
      break;
    case HK_OP_JUMP_IF_TRUE:
      {
        int32_t offset = read_word(&pc);
        hk_value_t val = slots[vm->stack_top];
        if (hk_is_truthy(val))
          pc = &code[offset];
        hk_value_release(val);
        --vm->stack_top;
      }
      break;
    case HK_OP_JUMP_IF_TRUE_OR_POP:
      {
        int32_t offset = read_word(&pc);
        hk_value_t val = slots[vm->stack_top];
        if (hk_is_truthy(val))
        {
          pc = &code[offset];
          break;
        }
        hk_value_release(val);
        --vm->stack_top;
      }
      break;
    case HK_OP_JUMP_IF_FALSE_OR_POP:
      {
        int32_t offset = read_word(&pc);
        hk_value_t val = slots[vm->stack_top];
        if (hk_is_falsey(val))
        {
          pc = &code[offset];
          break;
        }
        hk_value_release(val);
        --vm->stack_top;
      }
      break;
    case HK_OP_JUMP_IF_NOT_EQUAL:
      {
        int32_t offset = read_word(&pc);
        hk_value_t val1 = slots[vm->stack_top - 1];
        hk_value_t val2 = slots[vm->stack_top];
        if (hk_value_equal(val1, val2))
        {
          hk_value_release(val1);
          hk_value_release(val2);
          vm->stack_top -= 2;
          break;
        }
        pc = &code[offset];
        hk_value_release(val2);
        --vm->stack_top;
      }
      break;
    case HK_OP_EQUAL:
      do_equal(vm);
      break;
    case HK_OP_GREATER:
      if (do_greater(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_LESS:
      if (do_less(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NOT_EQUAL:
      do_not_equal(vm);
      break;
    case HK_OP_NOT_GREATER:
      if (do_not_greater(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NOT_LESS:
      if (do_not_less(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_BITWISE_OR:
      if (do_bitwise_or(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_BITWISE_XOR:
      if (do_bitwise_xor(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_BITWISE_AND:
      if (do_bitwise_and(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_LEFT_SHIFT:
      if (do_left_shift(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_RIGHT_SHIFT:
      if (do_right_shift(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_ADD:
      if (do_add(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_SUBTRACT:
      if (do_subtract(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_MULTIPLY:
      if (do_multiply(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_DIVIDE:
      if (do_divide(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_QUOTIENT:
      if (do_quotient(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_REMAINDER:
      if (do_remainder(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NEGATE:
      if (do_negate(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NOT:
      do_not(vm);
      break;
    case HK_OP_BITWISE_NOT:
      if (do_bitwise_not(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INCR:
      if (do_incr(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_DECR:
      if (do_decr(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CALL:
      if (do_call(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_LOAD_MODULE:
      if (load_module(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_RETURN:
      return HK_STATUS_OK;
    case HK_OP_RETURN_NIL:
      if (push(vm, HK_NIL_VALUE) == HK_STATUS_ERROR)
        goto error;
      return HK_STATUS_OK;
    }
  }
error:
  *line = hk_chunk_get_line(chunk, (int32_t) (pc - code));
  return HK_STATUS_ERROR;
}

static inline void discard_frame(hk_vm_t *vm, hk_value_t *slots)
{
  while (&vm->stack[vm->stack_top] >= slots)
    hk_value_release(vm->stack[vm->stack_top--]);
}

static inline void move_result(hk_vm_t *vm, hk_value_t *slots)
{
  slots[0] = vm->stack[vm->stack_top];
  --vm->stack_top;
  while (&vm->stack[vm->stack_top] > slots)
    hk_value_release(vm->stack[vm->stack_top--]);
}

void hk_vm_init(hk_vm_t *vm, int32_t min_capacity)
{
  int32_t capacity = min_capacity < HK_STACK_MIN_CAPACITY ? HK_STACK_MIN_CAPACITY : min_capacity;
  capacity = hk_power_of_two_ceil(capacity);
  vm->stack_end = capacity - 1;
  vm->stack_top = -1;
  vm->stack = (hk_value_t *) hk_allocate(sizeof(*vm->stack) * capacity);
  load_globals(vm);
  init_module_cache();
}

void hk_vm_free(hk_vm_t *vm)
{
  free_module_cache();
  hk_assert(vm->stack_top == num_globals() - 1, "stack must contain the globals");
  while (vm->stack_top > -1)
    hk_value_release(vm->stack[vm->stack_top--]);
  free(vm->stack);
}

int32_t hk_vm_push(hk_vm_t *vm, hk_value_t val)
{
  if (push(vm, val) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_incr_ref(val);
  return HK_STATUS_OK;
}

int32_t hk_vm_push_nil(hk_vm_t *vm)
{
  return push(vm, HK_NIL_VALUE);
}

int32_t hk_vm_push_bool(hk_vm_t *vm, bool data)
{
  return push(vm, data ? HK_TRUE_VALUE : HK_FALSE_VALUE);
}

int32_t hk_vm_push_float(hk_vm_t *vm, double data)
{
  return push(vm, hk_float_value(data));
}

int32_t hk_vm_push_string(hk_vm_t *vm, hk_string_t *str)
{
  if (push(vm, hk_string_value(str)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(str);
  return HK_STATUS_OK;
}

int32_t hk_vm_push_string_from_chars(hk_vm_t *vm, int32_t length, const char *chars)
{   
  hk_string_t *str = hk_string_from_chars(length, chars);
  if (hk_vm_push_string(vm, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int32_t hk_vm_push_string_from_stream(hk_vm_t *vm, FILE *stream, const char terminal)
{
  hk_string_t *str = hk_string_from_stream(stream, terminal);
  if (hk_vm_push_string(vm, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int32_t hk_vm_push_range(hk_vm_t *vm, hk_range_t *range)
{
  if (push(vm, hk_range_value(range)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(range);
  return HK_STATUS_OK;
}

int32_t hk_vm_push_array(hk_vm_t *vm, hk_array_t *arr)
{
  if (push(vm, hk_array_value(arr)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(arr);
  return HK_STATUS_OK;
}

int32_t hk_vm_push_struct(hk_vm_t *vm, hk_struct_t *ztruct)
{
  if (push(vm, hk_struct_value(ztruct)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(ztruct);
  return HK_STATUS_OK;
}

int32_t hk_vm_push_instance(hk_vm_t *vm, hk_instance_t *inst)
{
  if (push(vm, hk_instance_value(inst)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(inst);
  return HK_STATUS_OK;
}

int32_t hk_vm_push_iterator(hk_vm_t *vm, hk_iterator_t *it)
{
  if (push(vm, hk_iterator_value(it)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(it);
  return HK_STATUS_OK;
}

int32_t hk_vm_push_closure(hk_vm_t *vm, hk_closure_t *cl)
{
  if (push(vm, hk_closure_value(cl)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(cl);
  return HK_STATUS_OK;
}

int32_t hk_vm_push_native(hk_vm_t *vm, hk_native_t *native)
{
  if (push(vm, hk_native_value(native)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(native);
  return HK_STATUS_OK;
}

int32_t hk_vm_push_new_native(hk_vm_t *vm, const char *name, int32_t arity, int32_t (*call)(hk_vm_t *, hk_value_t *))
{
  hk_native_t *native = hk_native_new(hk_string_from_chars(-1, name), arity, call);
  if (hk_vm_push_native(vm, native) == HK_STATUS_ERROR)
  {
    hk_native_free(native);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int32_t hk_vm_push_userdata(hk_vm_t *vm, hk_userdata_t *udata)
{
  if (push(vm, hk_userdata_value(udata)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(udata);
  return HK_STATUS_OK;
}

int32_t hk_vm_array(hk_vm_t *vm, int32_t length)
{
  return do_array(vm, length);
}

int32_t hk_vm_struct(hk_vm_t *vm, int32_t length)
{
  return do_struct(vm, length);
}

int32_t hk_vm_instance(hk_vm_t *vm, int32_t num_args)
{
  return do_instance(vm, num_args);
}

int32_t hk_vm_construct(hk_vm_t *vm, int32_t length)
{
  return do_construct(vm, length);
}

void hk_vm_pop(hk_vm_t *vm)
{
  pop(vm);
}

int32_t hk_vm_call(hk_vm_t *vm, int32_t num_args)
{
  return do_call(vm, num_args);
}

int32_t hk_vm_check_type(hk_value_t *args, int32_t index, int32_t type)
{
  int32_t val_type = args[index].type;
  if (val_type != type)
  {
    hk_runtime_error("type error: argument #%d must be of the type %s, %s given", index,
      hk_type_name(type), hk_type_name(val_type));
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int32_t hk_vm_check_types(hk_value_t *args, int32_t index, int32_t num_types, int32_t types[])
{
  int32_t val_type = args[index].type;
  bool match = false;
  for (int32_t i = 0; i < num_types; ++i)
    if ((match = (val_type == types[i])))
      break;
  if (!match)
  {
    type_error(index, num_types, types, val_type);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int32_t hk_vm_check_bool(hk_value_t *args, int32_t index)
{
  return hk_vm_check_type(args, index, HK_TYPE_BOOL);
}

int32_t hk_vm_check_float(hk_value_t *args, int32_t index)
{
  return hk_vm_check_type(args, index, HK_TYPE_FLOAT);
}

int32_t hk_vm_check_int(hk_value_t *args, int32_t index)
{
  hk_value_t val = args[index];
  if (!hk_is_int(val))
  {
    hk_runtime_error("type error: argument #%d must be of the type int, %s given",
      index, hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int32_t hk_vm_check_string(hk_value_t *args, int32_t index)
{
  return hk_vm_check_type(args, index, HK_TYPE_STRING);
}

int32_t hk_vm_check_range(hk_value_t *args, int32_t index)
{
  return hk_vm_check_type(args, index, HK_TYPE_RANGE);
}

int32_t hk_vm_check_array(hk_value_t *args, int32_t index)
{
  return hk_vm_check_type(args, index, HK_TYPE_ARRAY);
}

int32_t hk_vm_check_struct(hk_value_t *args, int32_t index)
{
  return hk_vm_check_type(args, index, HK_TYPE_STRUCT);
}

int32_t hk_vm_check_instance(hk_value_t *args, int32_t index)
{
  return hk_vm_check_type(args, index, HK_TYPE_INSTANCE);
}

int32_t hk_vm_check_iterator(hk_value_t *args, int32_t index)
{
  return hk_vm_check_type(args, index, HK_TYPE_ITERATOR);
}

int32_t hk_vm_check_callable(hk_value_t *args, int32_t index)
{
  return hk_vm_check_type(args, index, HK_TYPE_CALLABLE);
}

int32_t hk_vm_check_userdata(hk_value_t *args, int32_t index)
{
  return hk_vm_check_type(args, index, HK_TYPE_USERDATA);
}
