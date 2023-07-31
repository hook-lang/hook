//
// The Hook Programming Language
// state.c
//

#include <hook/state.h>
#include <stdlib.h>
#include <math.h>
#include <hook/struct.h>
#include <hook/iterable.h>
#include <hook/memory.h>
#include <hook/status.h>
#include <hook/error.h>
#include <hook/utils.h>
#include "module.h"
#include "builtin.h"

static inline int push(HkState *state, HkValue val);
static inline void pop(HkState *state);
static inline int read_byte(uint8_t **pc);
static inline int read_word(uint8_t **pc);
static inline int do_range(HkState *state);
static inline int do_array(HkState *state, int length);
static inline int do_struct(HkState *state, int length);
static inline int do_instance(HkState *state, int num_args);
static inline int adjust_instance_args(HkState *state, int length, int num_args);
static inline int do_construct(HkState *state, int length);
static inline int do_iterator(HkState *state);
static inline int do_closure(HkState *state, HkFunction *fn);
static inline int do_unpack_array(HkState *state, int n);
static inline int do_unpack_struct(HkState *state, int n);
static inline int do_add_element(HkState *state);
static inline int do_get_element(HkState *state);
static inline void slice_string(HkState *state, HkValue *slot, HkString *str, HkRange *range);
static inline void slice_array(HkState *state, HkValue *slot, HkArray *arr, HkRange *range);
static inline int do_fetch_element(HkState *state);
static inline void do_set_element(HkState *state);
static inline int do_put_element(HkState *state);
static inline int do_delete_element(HkState *state);
static inline int do_inplace_add_element(HkState *state);
static inline int do_inplace_put_element(HkState *state);
static inline int do_inplace_delete_element(HkState *state);
static inline int do_get_field(HkState *state, HkString *name);
static inline int do_fetch_field(HkState *state, HkString *name);
static inline void do_set_field(HkState *state);
static inline int do_put_field(HkState *state, HkString *name);
static inline int do_inplace_put_field(HkState *state, HkString *name);
static inline void do_current(HkState *state);
static inline void do_next(HkState *state);
static inline void do_equal(HkState *state);
static inline int do_greater(HkState *state);
static inline int do_less(HkState *state);
static inline void do_not_equal(HkState *state);
static inline int do_not_greater(HkState *state);
static inline int do_not_less(HkState *state);
static inline int do_bitwise_or(HkState *state);
static inline int do_bitwise_xor(HkState *state);
static inline int do_bitwise_and(HkState *state);
static inline int do_left_shift(HkState *state);
static inline int do_right_shift(HkState *state);
static inline int do_add(HkState *state);
static inline int concat_strings(HkState *state, HkValue *slots, HkValue val1, HkValue val2);
static inline int concat_arrays(HkState *state, HkValue *slots, HkValue val1, HkValue val2);
static inline int do_subtract(HkState *state);
static inline int diff_arrays(HkState *state, HkValue *slots, HkValue val1, HkValue val2);
static inline int do_multiply(HkState *state);
static inline int do_divide(HkState *state);
static inline int do_quotient(HkState *state);
static inline int do_remainder(HkState *state);
static inline int do_negate(HkState *state);
static inline void do_not(HkState *state);
static inline int do_bitwise_not(HkState *state);
static inline int do_increment(HkState *state);
static inline int do_decrement(HkState *state);
static inline int do_call(HkState *state, int num_args);
static inline int adjust_call_args(HkState *state, int arity, int num_args);
static inline void print_trace(HkString *name, HkString *file, int line);
static inline int call_function(HkState *state, HkValue *locals, HkClosure *cl, int *line);
static inline void discard_frame(HkState *state, HkValue *slots);
static inline void move_result(HkState *state, HkValue *slots);

static inline int push(HkState *state, HkValue val)
{
  if (state->stackTop == state->stackEnd)
  {
    hk_runtime_error("stack overflow");
    return HK_STATUS_ERROR;
  }
  ++state->stackTop;
  state->stackSlots[state->stackTop] = val;
  return HK_STATUS_OK;
}

static inline void pop(HkState *state)
{
  hk_assert(state->stackTop > -1, "stack underflow");
  HkValue val = state->stackSlots[state->stackTop];
  --state->stackTop;
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

static inline int do_range(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: range must be of type number");
    return HK_STATUS_ERROR;
  }
  HkRange *range = hk_range_new(hk_as_number(val1), hk_as_number(val2));
  hk_incr_ref(range);
  slots[0] = hk_range_value(range);
  --state->stackTop;
  return HK_STATUS_OK;
}

static inline int do_array(HkState *state, int length)
{
  HkValue *slots = &state->stackSlots[state->stackTop - length + 1];
  HkArray *arr = hk_array_new_with_capacity(length);
  arr->length = length;
  for (int i = 0; i < length; ++i)
    arr->elements[i] = slots[i];
  state->stackTop -= length;
  if (push(state, hk_array_value(arr)) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  hk_incr_ref(arr);
  return HK_STATUS_OK;
}

static inline int do_struct(HkState *state, int length)
{
  HkValue *slots = &state->stackSlots[state->stackTop - length];
  HkValue val = slots[0];
  HkString *struct_name = hk_is_nil(val) ? NULL : hk_as_string(val);
  HkStruct *ztruct = hk_struct_new(struct_name);
  for (int i = 1; i <= length; ++i)
  {
    HkString *field_name = hk_as_string(slots[i]);
    if (!hk_struct_define_field(ztruct, field_name))
    {
      hk_runtime_error("field %.*s is already defined", field_name->length,
        field_name->chars);
      hk_struct_free(ztruct);
      return HK_STATUS_ERROR;
    }
  }
  for (int i = 1; i <= length; ++i)
    hk_decr_ref(hk_as_object(slots[i]));
  state->stackTop -= length;
  hk_incr_ref(ztruct);
  slots[0] = hk_struct_value(ztruct);
  if (struct_name)
    hk_decr_ref(struct_name);
  return HK_STATUS_OK;
}

static inline int do_instance(HkState *state, int num_args)
{
  HkValue *slots = &state->stackSlots[state->stackTop - num_args];
  HkValue val = slots[0];
  if (!hk_is_struct(val))
  {
    hk_runtime_error("type error: cannot use %s as a struct", hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  HkStruct *ztruct = hk_as_struct(val);
  int length = ztruct->length;
  adjust_instance_args(state, length, num_args);
  HkInstance *inst = hk_instance_new(ztruct);
  for (int i = 0; i < length; ++i)
    inst->values[i] = slots[i + 1];
  state->stackTop -= length;
  hk_incr_ref(inst);
  slots[0] = hk_instance_value(inst);
  hk_struct_release(ztruct);
  return HK_STATUS_OK;
}

static inline int adjust_instance_args(HkState *state, int length, int num_args)
{
  if (num_args > length)
  {
    do
    {
      pop(state);
      --num_args;
    }
    while (num_args > length);
    return HK_STATUS_OK;
  }
  while (num_args < length)
  {
    if (push(state, HK_NIL_VALUE) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    ++num_args;
  }
  return HK_STATUS_OK;
}

static inline int do_construct(HkState *state, int length)
{
  int n = length << 1;
  HkValue *slots = &state->stackSlots[state->stackTop - n];
  HkValue val = slots[0];
  HkString *struct_name = hk_is_nil(val) ? NULL : hk_as_string(val);
  HkStruct *ztruct = hk_struct_new(struct_name);
  for (int i = 1; i <= n; i += 2)
  {
    HkString *field_name = hk_as_string(slots[i]);
    if (hk_struct_define_field(ztruct, field_name))
      continue;
    hk_runtime_error("field %.*s is already defined", field_name->length,
      field_name->chars);
    hk_struct_free(ztruct);
    return HK_STATUS_ERROR;
  }
  for (int i = 1; i <= n; i += 2)
    hk_decr_ref(hk_as_object(slots[i]));
  HkInstance *inst = hk_instance_new(ztruct);
  for (int i = 2, j = 0; i <= n + 1; i += 2, ++j)
    inst->values[j] = slots[i];
  state->stackTop -= n;
  hk_incr_ref(inst);
  slots[0] = hk_instance_value(inst);
  if (struct_name)
    hk_decr_ref(struct_name);
  return HK_STATUS_OK;
}

static inline int do_iterator(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (hk_is_iterator(val))
    return HK_STATUS_OK;
  HkIterator *it = hk_new_iterator(val);
  if (!it)
  {
    hk_runtime_error("type error: value of type %s is not iterable", hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  hk_incr_ref(it);
  slots[0] = hk_iterator_value(it);
  hk_value_release(val);
  return HK_STATUS_OK;
}

static inline int do_closure(HkState *state, HkFunction *fn)
{
  int num_nonlocals = fn->num_nonlocals;
  HkValue *slots = &state->stackSlots[state->stackTop - num_nonlocals + 1];
  HkClosure *cl = hk_closure_new(fn);
  for (int i = 0; i < num_nonlocals; ++i)
    cl->nonlocals[i] = slots[i];
  state->stackTop -= num_nonlocals;
  if (push(state, hk_closure_value(cl)) == HK_STATUS_ERROR)
  {
    hk_closure_free(cl);
    return HK_STATUS_ERROR;
  }
  hk_incr_ref(cl);
  return HK_STATUS_OK;
}

static inline int do_unpack_array(HkState *state, int n)
{
  HkValue val = state->stackSlots[state->stackTop];
  if (!hk_is_array(val))
  {
    hk_runtime_error("type error: value of type %s is not an array",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  HkArray *arr = hk_as_array(val);
  --state->stackTop;
  int status = HK_STATUS_OK;
  for (int i = 0; i < n && i < arr->length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    if ((status = push(state, elem)) == HK_STATUS_ERROR)
      goto end;
    hk_value_incr_ref(elem);
  }
  for (int i = arr->length; i < n; ++i)
    if ((status = push(state, HK_NIL_VALUE)) == HK_STATUS_ERROR)
      break;
end:
  hk_array_release(arr);
  return status;
}

static inline int do_unpack_struct(HkState *state, int n)
{
  HkValue val = state->stackSlots[state->stackTop];
  if (!hk_is_instance(val))
  {
    hk_runtime_error("type error: value of type %s is not an instance of struct",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  HkInstance *inst = hk_as_instance(val);
  HkStruct *ztruct = inst->ztruct;
  HkValue *slots = &state->stackSlots[state->stackTop - n];
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
  --state->stackTop;
  hk_instance_release(inst);
  return HK_STATUS_OK;
}

static inline int do_add_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  HkArray *arr = hk_as_array(val1);
  HkArray *result = hk_array_add_element(arr, val2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr);  
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline int do_get_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
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
        hk_runtime_error("range error: index %d is out of bounds for string of length %d",
          index, str->length);
        return HK_STATUS_ERROR;
      }
      HkValue result = hk_string_value(hk_string_from_chars(1, &str->chars[(int) index]));
      hk_value_incr_ref(result);
      slots[0] = result;
      --state->stackTop;
      hk_string_release(str);
      return HK_STATUS_OK;
    }
    if (!hk_is_range(val2))
    {
      hk_runtime_error("type error: string cannot be indexed by %s", hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    slice_string(state, slots, str, hk_as_range(val2));
    return HK_STATUS_OK;
  }
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: %s cannot be indexed", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  HkArray *arr = hk_as_array(val1);
  if (hk_is_int(val2))
  {
    int64_t index = (int64_t) hk_as_number(val2);
    if (index < 0 || index >= arr->length)
    {
      hk_runtime_error("range error: index %d is out of bounds for array of length %d",
        index, arr->length);
      return HK_STATUS_ERROR;
    }
    HkValue result = hk_array_get_element(arr, (int) index);
    hk_value_incr_ref(result);
    slots[0] = result;
    --state->stackTop;
    hk_array_release(arr);
    return HK_STATUS_OK;
  }
  if (!hk_is_range(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  slice_array(state, slots, arr, hk_as_range(val2));
  return HK_STATUS_OK;
}

static inline void slice_string(HkState *state, HkValue *slot, HkString *str, HkRange *range)
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
    --state->stackTop;
    hk_range_release(range);
    return;
  }
  int length = end - start + 1;
  result = hk_string_from_chars(length, &str->chars[start]);
end:
  hk_incr_ref(result);
  *slot = hk_string_value(result);
  --state->stackTop;
  hk_string_release(str);
  hk_range_release(range);
}

static inline void slice_array(HkState *state, HkValue *slot, HkArray *arr, HkRange *range)
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
    --state->stackTop;
    hk_range_release(range);
    return;
  }
  int length = end - start + 1;
  result = hk_array_new_with_capacity(length);
  result->length = length;
  for (int i = start, j = 0; i <= end ; ++i, ++j)
  {
    HkValue elem = hk_array_get_element(arr, i);
    hk_value_incr_ref(elem);
    result->elements[j] = elem;
  }
end:
  hk_incr_ref(result);
  *slot = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr);
  hk_range_release(range);
}

static inline int do_fetch_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
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
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  HkValue elem = hk_array_get_element(arr, (int) index);
  if (push(state, elem) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_incr_ref(elem);
  return HK_STATUS_OK;
}

static inline void do_set_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 2];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  HkValue val3 = slots[2];
  HkArray *arr = hk_as_array(val1);
  int index = (int) hk_as_number(val2);
  HkArray *result = hk_array_set_element(arr, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  state->stackTop -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
}

static inline int do_put_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 2];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  HkValue val3 = slots[2];
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
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  HkArray *result = hk_array_set_element(arr, (int) index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  state->stackTop -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
  return HK_STATUS_OK;
}

static inline int do_delete_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
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
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  HkArray *result = hk_array_delete_element(arr, (int) index);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr);
  return HK_STATUS_OK;
}

static inline int do_inplace_add_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  HkArray *arr = hk_as_array(val1);
  if (arr->ref_count == 2)
  {
    hk_array_inplace_add_element(arr, val2);
    --state->stackTop;
    hk_value_decr_ref(val2);
    return HK_STATUS_OK;
  }
  HkArray *result = hk_array_add_element(arr, val2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr);
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline int do_inplace_put_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 2];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  HkValue val3 = slots[2];
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
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  if (arr->ref_count == 2)
  {
    hk_array_inplace_set_element(arr, (int) index, val3);
    state->stackTop -= 2;
    hk_value_decr_ref(val3);
    return HK_STATUS_OK;
  }
  HkArray *result = hk_array_set_element(arr, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  state->stackTop -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
  return HK_STATUS_OK;
}

static inline int do_inplace_delete_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
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
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  if (arr->ref_count == 2)
  {
    hk_array_inplace_delete_element(arr, (int) index);
    --state->stackTop;
    return HK_STATUS_OK;
  }
  HkArray *result = hk_array_delete_element(arr, index);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr);
  return HK_STATUS_OK;
}

static inline int do_get_field(HkState *state, HkString *name)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_instance(val))
  {
    hk_runtime_error("type error: cannot use %s as an instance of struct",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  HkInstance *inst = hk_as_instance(val);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  HkValue value = hk_instance_get_field(inst, index);
  hk_value_incr_ref(value);
  slots[0] = value;
  hk_instance_release(inst);
  return HK_STATUS_OK;
}

static inline int do_fetch_field(HkState *state, HkString *name)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_instance(val))
  {
    hk_runtime_error("type error: cannot use %s as an instance of struct",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  HkInstance *inst = hk_as_instance(val);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  if (push(state, hk_number_value(index)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkValue value = hk_instance_get_field(inst, index);
  if (push(state, value) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_incr_ref(value);
  return HK_STATUS_OK;
}

static inline void do_set_field(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 2];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  HkValue val3 = slots[2];
  HkInstance *inst = hk_as_instance(val1);
  int index = (int) hk_as_number(val2);
  HkInstance *result = hk_instance_set_field(inst, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  state->stackTop -= 2;
  hk_instance_release(inst);
  hk_value_decr_ref(val3);
}

static inline int do_put_field(HkState *state, HkString *name)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_instance(val1))
  {
    hk_runtime_error("type error: cannot use %s as an instance of struct",
      hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  HkInstance *inst = hk_as_instance(val1);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  HkInstance *result = hk_instance_set_field(inst, index, val2);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  --state->stackTop;
  hk_instance_release(inst);
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline int do_inplace_put_field(HkState *state, HkString *name)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_instance(val1))
  {
    hk_runtime_error("type error: cannot use %s as an instance of struct",
      hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  HkInstance *inst = hk_as_instance(val1);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  if (inst->ref_count == 2)
  {
    hk_instance_inplace_set_field(inst, index, val2);
    --state->stackTop;
    hk_value_decr_ref(val2);
    return HK_STATUS_OK;
  }
  HkInstance *result = hk_instance_set_field(inst, index, val2);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  --state->stackTop;
  hk_instance_release(inst);
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline void do_current(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val = slots[1];
  HkIterator *it = hk_as_iterator(val);
  HkValue result = hk_iterator_get_current(it);
  hk_value_incr_ref(result);
  hk_value_release(slots[0]);
  slots[0] = result;
}

static inline void do_next(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  HkIterator *it = hk_as_iterator(val);
  if (it->ref_count == 2)
  {
    hk_iterator_inplace_next(it);
    return;
  }
  HkIterator *result = hk_iterator_next(it);
  hk_incr_ref(result);
  slots[0] = hk_iterator_value(result);
  hk_iterator_release(it);
}

static inline void do_equal(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  slots[0] = hk_value_equal(val1, val2) ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --state->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline int do_greater(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  if (hk_state_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result > 0 ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --state->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline int do_less(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  if (hk_state_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result < 0 ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --state->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline void do_not_equal(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  slots[0] = hk_value_equal(val1, val2) ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --state->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline int do_not_greater(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  if (hk_state_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result > 0 ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --state->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline int do_not_less(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  if (hk_state_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result < 0 ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --state->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline int do_bitwise_or(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot apply `bitwise or` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ((int64_t) hk_as_number(val1)) | ((int64_t) hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
  return HK_STATUS_OK;
}

static inline int do_bitwise_xor(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot apply `bitwise xor` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ((int64_t) hk_as_number(val1)) ^ ((int64_t) hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
  return HK_STATUS_OK;
}

static inline int do_bitwise_and(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot apply `bitwise and` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ((int64_t) hk_as_number(val1)) & ((int64_t) hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
  return HK_STATUS_OK;
}

static inline int do_left_shift(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot apply `left shift` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ((int64_t) hk_as_number(val1)) << ((int64_t) hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
  return HK_STATUS_OK;
}

static inline int do_right_shift(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot apply `right shift` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ((int64_t) hk_as_number(val1)) >> ((int64_t) hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
  return HK_STATUS_OK;
}

static inline int do_add(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (hk_is_number(val1))
  {
    if (!hk_is_number(val2))
    {
      hk_runtime_error("type error: cannot add %s to number", hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    double data = hk_as_number(val1) + hk_as_number(val2);
    slots[0] = hk_number_value(data);
    --state->stackTop;
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
    return concat_strings(state, slots, val1, val2);
  }
  if (hk_is_array(val1))
  {
    if (!hk_is_array(val2))
    {
      hk_runtime_error("type error: cannot concatenate array and %s",
        hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    return concat_arrays(state, slots, val1, val2);
  }
  hk_runtime_error("type error: cannot add %s to %s", hk_type_name(val2.type),
    hk_type_name(val1.type));
  return HK_STATUS_ERROR;
}

static inline int concat_strings(HkState *state, HkValue *slots, HkValue val1, HkValue val2)
{
  HkString *str1 = hk_as_string(val1);
  if (!str1->length)
  {
    slots[0] = val2;
    --state->stackTop;
    hk_string_release(str1);
    return HK_STATUS_OK;
  }
  HkString *str2 = hk_as_string(val2);
  if (!str2->length)
  {
    --state->stackTop;
    hk_string_release(str2);
    return HK_STATUS_OK;
  }
  if (str1->ref_count == 1)
  {
    hk_string_inplace_concat(str1, str2);
    --state->stackTop;
    hk_string_release(str2);
    return HK_STATUS_OK;
  }
  HkString *result = hk_string_concat(str1, str2);
  hk_incr_ref(result);
  slots[0] = hk_string_value(result);
  --state->stackTop;
  hk_string_release(str1);
  hk_string_release(str2);
  return HK_STATUS_OK;
}

static inline int concat_arrays(HkState *state, HkValue *slots, HkValue val1, HkValue val2)
{
  HkArray *arr1 = hk_as_array(val1);
  if (!arr1->length)
  {
    slots[0] = val2;
    --state->stackTop;
    hk_array_release(arr1);
    return HK_STATUS_OK;
  }
  HkArray *arr2 = hk_as_array(val2);
  if (!arr2->length)
  {
    --state->stackTop;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  if (arr1->ref_count == 1)
  {
    hk_array_inplace_concat(arr1, arr2);
    --state->stackTop;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  HkArray *result = hk_array_concat(arr1, arr2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr1);
  hk_array_release(arr2);
  return HK_STATUS_OK;
}

static inline int do_subtract(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (hk_is_number(val1))
  {
    if (!hk_is_number(val2))
    {
      hk_runtime_error("type error: cannot subtract %s from number",
        hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    double data = hk_as_number(val1) - hk_as_number(val2);
    slots[0] = hk_number_value(data);
    --state->stackTop;
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
    return diff_arrays(state, slots, val1, val2);
  }
  hk_runtime_error("type error: cannot subtract %s from %s", hk_type_name(val2.type),
    hk_type_name(val1.type));
  return HK_STATUS_ERROR;
}

static inline int diff_arrays(HkState *state, HkValue *slots, HkValue val1, HkValue val2)
{
  HkArray *arr1 = hk_as_array(val1);
  HkArray *arr2 = hk_as_array(val2);
  if (!arr1->length || !arr2->length)
  {
    --state->stackTop;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  if (arr1->ref_count == 1)
  {
    hk_array_inplace_diff(arr1, arr2);
    --state->stackTop;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  HkArray *result = hk_array_diff(arr1, arr2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr1);
  hk_array_release(arr2);
  return HK_STATUS_OK;
}

static inline int do_multiply(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot multiply %s to %s", hk_type_name(val2.type),
      hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  double data = hk_as_number(val1) * hk_as_number(val2);
  slots[0] = hk_number_value(data);
  --state->stackTop;
  return HK_STATUS_OK;
}

static inline int do_divide(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot divide %s by %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  double data = hk_as_number(val1) / hk_as_number(val2);
  slots[0] = hk_number_value(data);
  --state->stackTop;
  return HK_STATUS_OK;
}

static inline int do_quotient(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot apply `quotient` between %s and %s",
      hk_type_name(val1.type), hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  double data = floor(hk_as_number(val1) / hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
  return HK_STATUS_OK;
}

static inline int do_remainder(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot apply `remainder` between %s and %s",
      hk_type_name(val1.type), hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  double data = fmod(hk_as_number(val1), hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
  return HK_STATUS_OK;
}

static inline int do_negate(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_runtime_error("type error: cannot apply `negate` to %s", hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  double data = -hk_as_number(val);
  slots[0] = hk_number_value(data);
  return HK_STATUS_OK;
}

static inline void do_not(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  slots[0] = hk_is_falsey(val) ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  hk_value_release(val);
}

static inline int do_bitwise_not(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_runtime_error("type error: cannot apply `bitwise not` to %s", hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  int64_t data = ~((int64_t) hk_as_number(val));
  slots[0] = hk_number_value(data);
  return HK_STATUS_OK;
}

static inline int do_increment(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_runtime_error("type error: cannot increment value of type %s",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  ++slots[0].as.number;
  return HK_STATUS_OK;
}

static inline int do_decrement(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_runtime_error("type error: cannot decrement value of type %s",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  --slots[0].as.number;
  return HK_STATUS_OK;
}

static inline int do_call(HkState *state, int num_args)
{
  HkValue *slots = &state->stackSlots[state->stackTop - num_args];
  HkValue val = slots[0];
  if (!hk_is_callable(val))
  {
    hk_runtime_error("type error: cannot call value of type %s",
      hk_type_name(val.type));
    discard_frame(state, slots);
    return HK_STATUS_ERROR;
  }
  if (hk_is_native(val))
  {
    HkNative *native = hk_as_native(val);
    if (adjust_call_args(state, native->arity, num_args) == HK_STATUS_ERROR)
    {
      discard_frame(state, slots);
      return HK_STATUS_ERROR;
    }
    int status;
    if ((status = native->call(state, slots)) != HK_STATUS_OK)
    {
      if (status != HK_STATUS_NO_TRACE)
        print_trace(native->name, NULL, 0);
      discard_frame(state, slots);
      return HK_STATUS_ERROR;
    }
    hk_native_release(native);
    move_result(state, slots);
    return HK_STATUS_OK;
  }
  HkClosure *cl = hk_as_closure(val);
  HkFunction *fn = cl->fn;
  if (adjust_call_args(state, fn->arity, num_args) == HK_STATUS_ERROR)
  {
    discard_frame(state, slots);
    return HK_STATUS_ERROR;
  }
  int line;
  if (call_function(state, slots, cl, &line) == HK_STATUS_ERROR)
  {
    print_trace(fn->name, fn->file, line);
    discard_frame(state, slots);
    return HK_STATUS_ERROR;
  }
  hk_closure_release(cl);
  move_result(state, slots);
  return HK_STATUS_OK;
}

static inline int adjust_call_args(HkState *state, int arity,int num_args)
{
  if (num_args >= arity)
    return HK_STATUS_OK;
  while (num_args < arity)
  {
    if (push(state, HK_NIL_VALUE) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    ++num_args;
  }
  return HK_STATUS_OK;
}

static inline void print_trace(HkString *name, HkString *file, int line)
{
  char *name_chars = name ? name->chars : "<anonymous>";
  if (file)
  {
    fprintf(stderr, "  at %s() in %.*s:%d\n", name_chars, file->length, file->chars, line);
    return;
  }
  fprintf(stderr, "  at %s() in <native>\n", name_chars);
}

static inline int call_function(HkState *state, HkValue *locals, HkClosure *cl, int *line)
{
  HkValue *slots = state->stackSlots;
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
      if (push(state, HK_NIL_VALUE) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_FALSE:
      if (push(state, HK_FALSE_VALUE) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_TRUE:
      if (push(state, HK_TRUE_VALUE) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INT:
      if (push(state, hk_number_value(read_word(&pc))) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CONSTANT:
      {
        HkValue val = consts[read_byte(&pc)];
        if (push(state, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_RANGE:
      if (do_range(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_ARRAY:
      if (do_array(state, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_STRUCT:
      if (do_struct(state, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INSTANCE:
      if (do_instance(state, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CONSTRUCT:
      if (do_construct(state, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_ITERATOR:
      if (do_iterator(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CLOSURE:
      if (do_closure(state, functions[read_byte(&pc)]) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_UNPACK_ARRAY:
      if (do_unpack_array(state, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_UNPACK_STRUCT:
      if (do_unpack_struct(state, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_POP:
      hk_value_release(slots[state->stackTop--]);
      break;
    case HK_OP_GLOBAL:
      {
        HkValue val = slots[read_byte(&pc)];
        if (push(state, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_NONLOCAL:
      {
        HkValue val = nonlocals[read_byte(&pc)];
        if (push(state, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_LOAD:
      {
        HkValue val = locals[read_byte(&pc)];
        if (push(state, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_STORE:
      {
        int index = read_byte(&pc);
        HkValue val = slots[state->stackTop];
        --state->stackTop;
        hk_value_release(locals[index]);
        locals[index] = val;
      }
      break;
    case HK_OP_ADD_ELEMENT:
      if (do_add_element(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_GET_ELEMENT:
      if (do_get_element(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_FETCH_ELEMENT:
      if (do_fetch_element(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_SET_ELEMENT:
      do_set_element(state);
      break;
    case HK_OP_PUT_ELEMENT:
      if (do_put_element(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_DELETE_ELEMENT:
      if (do_delete_element(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_ADD_ELEMENT:
      if (do_inplace_add_element(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_PUT_ELEMENT:
      if (do_inplace_put_element(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_DELETE_ELEMENT:
      if (do_inplace_delete_element(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_GET_FIELD:
      if (do_get_field(state, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_FETCH_FIELD:
      if (do_fetch_field(state, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_SET_FIELD:
      do_set_field(state);
      break;
    case HK_OP_PUT_FIELD:
      if (do_put_field(state, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_PUT_FIELD:
      if (do_inplace_put_field(state, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CURRENT:
      do_current(state);
      break;
    case HK_OP_JUMP:
      pc = &code[read_word(&pc)];
      break;
    case HK_OP_JUMP_IF_FALSE:
      {
        int offset = read_word(&pc);
        HkValue val = slots[state->stackTop];
        if (hk_is_falsey(val))
          pc = &code[offset];
        hk_value_release(val);
        --state->stackTop;
      }
      break;
    case HK_OP_JUMP_IF_TRUE:
      {
        int offset = read_word(&pc);
        HkValue val = slots[state->stackTop];
        if (hk_is_truthy(val))
          pc = &code[offset];
        hk_value_release(val);
        --state->stackTop;
      }
      break;
    case HK_OP_JUMP_IF_TRUE_OR_POP:
      {
        int offset = read_word(&pc);
        HkValue val = slots[state->stackTop];
        if (hk_is_truthy(val))
        {
          pc = &code[offset];
          break;
        }
        hk_value_release(val);
        --state->stackTop;
      }
      break;
    case HK_OP_JUMP_IF_FALSE_OR_POP:
      {
        int offset = read_word(&pc);
        HkValue val = slots[state->stackTop];
        if (hk_is_falsey(val))
        {
          pc = &code[offset];
          break;
        }
        hk_value_release(val);
        --state->stackTop;
      }
      break;
    case HK_OP_JUMP_IF_NOT_EQUAL:
      {
        int offset = read_word(&pc);
        HkValue val1 = slots[state->stackTop - 1];
        HkValue val2 = slots[state->stackTop];
        if (hk_value_equal(val1, val2))
        {
          hk_value_release(val1);
          hk_value_release(val2);
          state->stackTop -= 2;
          break;
        }
        pc = &code[offset];
        hk_value_release(val2);
        --state->stackTop;
      }
      break;
    case HK_OP_JUMP_IF_NOT_VALID:
      {
        int offset = read_word(&pc);
        HkValue val = slots[state->stackTop];
        HkIterator *it = hk_as_iterator(val);
        if (!hk_iterator_is_valid(it))
          pc = &code[offset];
      }
      break;
    case HK_OP_NEXT:
      do_next(state);
      break;
    case HK_OP_EQUAL:
      do_equal(state);
      break;
    case HK_OP_GREATER:
      if (do_greater(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_LESS:
      if (do_less(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NOT_EQUAL:
      do_not_equal(state);
      break;
    case HK_OP_NOT_GREATER:
      if (do_not_greater(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NOT_LESS:
      if (do_not_less(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_BITWISE_OR:
      if (do_bitwise_or(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_BITWISE_XOR:
      if (do_bitwise_xor(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_BITWISE_AND:
      if (do_bitwise_and(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_LEFT_SHIFT:
      if (do_left_shift(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_RIGHT_SHIFT:
      if (do_right_shift(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_ADD:
      if (do_add(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_SUBTRACT:
      if (do_subtract(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_MULTIPLY:
      if (do_multiply(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_DIVIDE:
      if (do_divide(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_QUOTIENT:
      if (do_quotient(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_REMAINDER:
      if (do_remainder(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NEGATE:
      if (do_negate(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NOT:
      do_not(state);
      break;
    case HK_OP_BITWISE_NOT:
      if (do_bitwise_not(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INCREMENT:
      if (do_increment(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_DECREMENT:
      if (do_decrement(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CALL:
      if (do_call(state, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_LOAD_MODULE:
      if (load_module(state) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_RETURN:
      return HK_STATUS_OK;
    case HK_OP_RETURN_NIL:
      if (push(state, HK_NIL_VALUE) == HK_STATUS_ERROR)
        goto error;
      return HK_STATUS_OK;
    }
  }
error:
  *line = hk_chunk_get_line(chunk, (int) (pc - code));
  return HK_STATUS_ERROR;
}

static inline void discard_frame(HkState *state, HkValue *slots)
{
  while (&state->stackSlots[state->stackTop] >= slots)
    hk_value_release(state->stackSlots[state->stackTop--]);
}

static inline void move_result(HkState *state, HkValue *slots)
{
  slots[0] = state->stackSlots[state->stackTop];
  --state->stackTop;
  while (&state->stackSlots[state->stackTop] > slots)
    hk_value_release(state->stackSlots[state->stackTop--]);
}

void hk_state_init(HkState *state, int minCapacity)
{
  int capacity = minCapacity < HK_STACK_MIN_CAPACITY ? HK_STACK_MIN_CAPACITY : minCapacity;
  capacity = hk_power_of_two_ceil(capacity);
  state->stackEnd = capacity - 1;
  state->stackTop = -1;
  state->stackSlots = (HkValue *) hk_allocate(sizeof(*state->stackSlots) * capacity);
  load_globals(state);
  init_module_cache();
}

void hk_state_free(HkState *state)
{
  free_module_cache();
  hk_assert(state->stackTop == num_globals() - 1, "stack must contain the globals");
  while (state->stackTop > -1)
    hk_value_release(state->stackSlots[state->stackTop--]);
  free(state->stackSlots);
}

int hk_state_push(HkState *state, HkValue val)
{
  if (push(state, val) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_incr_ref(val);
  return HK_STATUS_OK;
}

int hk_state_push_nil(HkState *state)
{
  return push(state, HK_NIL_VALUE);
}

int hk_state_push_bool(HkState *state, bool data)
{
  return push(state, data ? HK_TRUE_VALUE : HK_FALSE_VALUE);
}

int hk_state_push_number(HkState *state, double data)
{
  return push(state, hk_number_value(data));
}

int hk_state_push_string(HkState *state, HkString *str)
{
  if (push(state, hk_string_value(str)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(str);
  return HK_STATUS_OK;
}

int hk_state_push_string_from_chars(HkState *state, int length, const char *chars)
{   
  HkString *str = hk_string_from_chars(length, chars);
  if (hk_state_push_string(state, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_state_push_string_from_stream(HkState *state, FILE *stream, const char terminal)
{
  HkString *str = hk_string_from_stream(stream, terminal);
  if (hk_state_push_string(state, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_state_push_range(HkState *state, HkRange *range)
{
  if (push(state, hk_range_value(range)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(range);
  return HK_STATUS_OK;
}

int hk_state_push_array(HkState *state, HkArray *arr)
{
  if (push(state, hk_array_value(arr)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(arr);
  return HK_STATUS_OK;
}

int hk_state_push_struct(HkState *state, HkStruct *ztruct)
{
  if (push(state, hk_struct_value(ztruct)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(ztruct);
  return HK_STATUS_OK;
}

int hk_state_push_instance(HkState *state, HkInstance *inst)
{
  if (push(state, hk_instance_value(inst)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(inst);
  return HK_STATUS_OK;
}

int hk_state_push_iterator(HkState *state, HkIterator *it)
{
  if (push(state, hk_iterator_value(it)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(it);
  return HK_STATUS_OK;
}

int hk_state_push_closure(HkState *state, HkClosure *cl)
{
  if (push(state, hk_closure_value(cl)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(cl);
  return HK_STATUS_OK;
}

int hk_state_push_native(HkState *state, HkNative *native)
{
  if (push(state, hk_native_value(native)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(native);
  return HK_STATUS_OK;
}

int hk_state_push_new_native(HkState *state, const char *name, int arity, int (*call)(HkState *, HkValue *))
{
  HkNative *native = hk_native_new(hk_string_from_chars(-1, name), arity, call);
  if (hk_state_push_native(state, native) == HK_STATUS_ERROR)
  {
    hk_native_free(native);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_state_push_userdata(HkState *state, HkUserdata *udata)
{
  if (push(state, hk_userdata_value(udata)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(udata);
  return HK_STATUS_OK;
}

int hk_state_array(HkState *state, int length)
{
  return do_array(state, length);
}

int hk_state_struct(HkState *state, int length)
{
  return do_struct(state, length);
}

int hk_state_instance(HkState *state, int num_args)
{
  return do_instance(state, num_args);
}

int hk_state_construct(HkState *state, int length)
{
  return do_construct(state, length);
}

void hk_state_pop(HkState *state)
{
  pop(state);
}

int hk_state_call(HkState *state, int num_args)
{
  return do_call(state, num_args);
}

int hk_state_compare(HkValue val1, HkValue val2, int *result)
{
  if (!hk_is_comparable(val1))
  {
    hk_runtime_error("type error: value of type %s is not comparable", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  if (val1.type != val2.type)
  {
    hk_runtime_error("type error: cannot compare %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  hk_assert(hk_value_compare(val1, val2, result), "hk_value_compare failed");
  return HK_STATUS_OK; 
}
