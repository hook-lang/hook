//
// The Hook Programming Language
// state.c
//

#include <hook/state.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <hook/struct.h>
#include <hook/iterable.h>
#include <hook/memory.h>
#include <hook/utils.h>
#include "module.h"
#include "builtin.h"

static inline void type_error(HkState *state, int index, int numTypes, HkType types[],
  HkType valType);
static inline void push(HkState *state, HkValue val);
static inline void pop(HkState *state);
static inline int read_byte(uint8_t **pc);
static inline int read_word(uint8_t **pc);
static inline void do_range(HkState *state);
static inline void do_array(HkState *state, int length);
static inline void do_struct(HkState *state, int length);
static inline void do_instance(HkState *state, int num_args);
static inline void adjust_instance_args(HkState *state, int length, int num_args);
static inline void do_construct(HkState *state, int length);
static inline void do_iterator(HkState *state);
static inline void do_closure(HkState *state, HkFunction *fn);
static inline void do_unpack_array(HkState *state, int n);
static inline void do_unpack_struct(HkState *state, int n);
static inline void do_add_element(HkState *state);
static inline void do_get_element(HkState *state);
static inline void slice_string(HkState *state, HkValue *slot, HkString *str, HkRange *range);
static inline void slice_array(HkState *state, HkValue *slot, HkArray *arr, HkRange *range);
static inline void do_fetch_element(HkState *state);
static inline void do_set_element(HkState *state);
static inline void do_put_element(HkState *state);
static inline void do_delete_element(HkState *state);
static inline void do_inplace_add_element(HkState *state);
static inline void do_inplace_put_element(HkState *state);
static inline void do_inplace_delete_element(HkState *state);
static inline void do_get_field(HkState *state, HkString *name);
static inline void do_fetch_field(HkState *state, HkString *name);
static inline void do_set_field(HkState *state);
static inline void do_put_field(HkState *state, HkString *name);
static inline void do_inplace_put_field(HkState *state, HkString *name);
static inline void do_current(HkState *state);
static inline void do_next(HkState *state);
static inline void do_equal(HkState *state);
static inline void do_greater(HkState *state);
static inline void do_less(HkState *state);
static inline void do_not_equal(HkState *state);
static inline void do_not_greater(HkState *state);
static inline void do_not_less(HkState *state);
static inline void do_bitwise_or(HkState *state);
static inline void do_bitwise_xor(HkState *state);
static inline void do_bitwise_and(HkState *state);
static inline void do_left_shift(HkState *state);
static inline void do_right_shift(HkState *state);
static inline void do_add(HkState *state);
static inline void concat_strings(HkState *state, HkValue *slots, HkValue val1, HkValue val2);
static inline void concat_arrays(HkState *state, HkValue *slots, HkValue val1, HkValue val2);
static inline void do_subtract(HkState *state);
static inline void diff_arrays(HkState *state, HkValue *slots, HkValue val1, HkValue val2);
static inline void do_multiply(HkState *state);
static inline void do_divide(HkState *state);
static inline void do_quotient(HkState *state);
static inline void do_remainder(HkState *state);
static inline void do_negate(HkState *state);
static inline void do_not(HkState *state);
static inline void do_bitwise_not(HkState *state);
static inline void do_increment(HkState *state);
static inline void do_decrement(HkState *state);
static inline void do_call(HkState *state, int num_args);
static inline void adjust_call_args(HkState *state, int arity, int num_args);
static inline void print_trace(HkString *name, HkString *file, int line);
static inline void call_function(HkState *state, HkValue *locals, HkClosure *cl, int *line);
static inline void discard_frame(HkState *state, HkValue *slots);
static inline void move_result(HkState *state, HkValue *slots);

static inline void type_error(HkState *state, int index, int numTypes, HkType types[],
  HkType valType)
{
  hk_assert(numTypes > 0, "numTypes must be greater than 0");
  state->status = HK_STATE_STATUS_ERROR;
  fprintf(stderr, "runtime error: type error: argument #%d must be of the type %s",
    index, hk_type_name(types[0]));
  for (int i = 1; i < numTypes; ++i)
    fprintf(stderr, "|%s", hk_type_name(types[i]));
  fprintf(stderr, ", %s given\n", hk_type_name(valType));
}

static inline void push(HkState *state, HkValue val)
{
  if (state->stackTop == state->stackEnd)
  {
    hk_state_runtime_error(state, "stack overflow");
    return;
  }
  ++state->stackTop;
  state->stackSlots[state->stackTop] = val;
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

static inline void do_range(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_state_runtime_error(state, "type error: range must be of type number");
    return;
  }
  HkRange *range = hk_range_new(hk_as_number(val1), hk_as_number(val2));
  hk_incr_ref(range);
  slots[0] = hk_range_value(range);
  --state->stackTop;
}

static inline void do_array(HkState *state, int length)
{
  HkValue *slots = &state->stackSlots[state->stackTop - length + 1];
  HkArray *arr = hk_array_new_with_capacity(length);
  arr->length = length;
  for (int i = 0; i < length; ++i)
    arr->elements[i] = slots[i];
  state->stackTop -= length;
  push(state, hk_array_value(arr));
  if (!hk_state_is_ok(state))
  {
    hk_array_free(arr);
    return;
  }
  hk_incr_ref(arr);
}

static inline void do_struct(HkState *state, int length)
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
      hk_state_runtime_error(state, "field %.*s is already defined", field_name->length,
        field_name->chars);
      hk_struct_free(ztruct);
      return;
    }
  }
  for (int i = 1; i <= length; ++i)
    hk_decr_ref(hk_as_object(slots[i]));
  state->stackTop -= length;
  hk_incr_ref(ztruct);
  slots[0] = hk_struct_value(ztruct);
  if (struct_name)
    hk_decr_ref(struct_name);
}

static inline void do_instance(HkState *state, int num_args)
{
  HkValue *slots = &state->stackSlots[state->stackTop - num_args];
  HkValue val = slots[0];
  if (!hk_is_struct(val))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as a struct", hk_type_name(val.type));
    return;
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
}

static inline void adjust_instance_args(HkState *state, int length, int num_args)
{
  if (num_args > length)
  {
    do
    {
      pop(state);
      --num_args;
    }
    while (num_args > length);
    return;
  }
  while (num_args < length)
  {
    push(state, HK_NIL_VALUE);
    hk_return_if_not_ok(state);
    ++num_args;
  }
}

static inline void do_construct(HkState *state, int length)
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
    hk_state_runtime_error(state, "field %.*s is already defined", field_name->length,
      field_name->chars);
    hk_struct_free(ztruct);
    return;
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
}

static inline void do_iterator(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (hk_is_iterator(val))
    return;
  HkIterator *it = hk_new_iterator(val);
  if (!it)
  {
    hk_state_runtime_error(state, "type error: value of type %s is not iterable", hk_type_name(val.type));
    return;
  }
  hk_incr_ref(it);
  slots[0] = hk_iterator_value(it);
  hk_value_release(val);
}

static inline void do_closure(HkState *state, HkFunction *fn)
{
  int numNonlocals = fn->numNonlocals;
  HkValue *slots = &state->stackSlots[state->stackTop - numNonlocals + 1];
  HkClosure *cl = hk_closure_new(fn);
  for (int i = 0; i < numNonlocals; ++i)
    cl->nonlocals[i] = slots[i];
  state->stackTop -= numNonlocals;
  push(state, hk_closure_value(cl));
  if (!hk_state_is_ok(state))
  {
    hk_closure_free(cl);
    return;
  }
  hk_incr_ref(cl);
}

static inline void do_unpack_array(HkState *state, int n)
{
  HkValue val = state->stackSlots[state->stackTop];
  if (!hk_is_array(val))
  {
    hk_state_runtime_error(state, "type error: value of type %s is not an array",
      hk_type_name(val.type));
    return;
  }
  HkArray *arr = hk_as_array(val);
  --state->stackTop;
  for (int i = 0; i < n && i < arr->length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    push(state, elem);
    if (!hk_state_is_ok(state))
      goto end;
    hk_value_incr_ref(elem);
  }
  for (int i = arr->length; i < n; ++i)
  {
    push(state, HK_NIL_VALUE);
    if (!hk_state_is_ok(state))
      break;
  }
end:
  hk_array_release(arr);
}

static inline void do_unpack_struct(HkState *state, int n)
{
  HkValue val = state->stackSlots[state->stackTop];
  if (!hk_is_instance(val))
  {
    hk_state_runtime_error(state, "type error: value of type %s is not an instance of struct",
      hk_type_name(val.type));
    return;
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
}

static inline void do_add_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  HkArray *result = hk_array_add_element(arr, val2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr);  
  hk_value_decr_ref(val2);
}

static inline void do_get_element(HkState *state)
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
        hk_state_runtime_error(state, "range error: index %d is out of bounds for string of length %d",
          index, str->length);
        return;
      }
      HkValue result = hk_string_value(hk_string_from_chars(1, &str->chars[(int) index]));
      hk_value_incr_ref(result);
      slots[0] = result;
      --state->stackTop;
      hk_string_release(str);
      return;
    }
    if (!hk_is_range(val2))
    {
      hk_state_runtime_error(state, "type error: string cannot be indexed by %s", hk_type_name(val2.type));
      return;
    }
    slice_string(state, slots, str, hk_as_range(val2));
    return;
  }
  if (!hk_is_array(val1))
  {
    hk_state_runtime_error(state, "type error: %s cannot be indexed", hk_type_name(val1.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  if (hk_is_int(val2))
  {
    int64_t index = (int64_t) hk_as_number(val2);
    if (index < 0 || index >= arr->length)
    {
      hk_state_runtime_error(state, "range error: index %d is out of bounds for array of length %d",
        index, arr->length);
      return;
    }
    HkValue result = hk_array_get_element(arr, (int) index);
    hk_value_incr_ref(result);
    slots[0] = result;
    --state->stackTop;
    hk_array_release(arr);
    return;
  }
  if (!hk_is_range(val2))
  {
    hk_state_runtime_error(state, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  slice_array(state, slots, arr, hk_as_range(val2));
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

static inline void do_fetch_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  if (!hk_is_int(val2))
  {
    hk_state_runtime_error(state, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_state_runtime_error(state, "range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return;
  }
  HkValue elem = hk_array_get_element(arr, (int) index);
  push(state, elem);
  hk_return_if_not_ok(state);
  hk_value_incr_ref(elem);
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

static inline void do_put_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 2];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  HkValue val3 = slots[2];
  if (!hk_is_array(val1))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  if (!hk_is_int(val2))
  {
    hk_state_runtime_error(state, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_state_runtime_error(state, "range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return;
  }
  HkArray *result = hk_array_set_element(arr, (int) index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  state->stackTop -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
}

static inline void do_delete_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  if (!hk_is_int(val2))
  {
    hk_state_runtime_error(state, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_state_runtime_error(state, "range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return;
  }
  HkArray *result = hk_array_delete_element(arr, (int) index);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr);
}

static inline void do_inplace_add_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  if (arr->refCount == 2)
  {
    hk_array_inplace_add_element(arr, val2);
    --state->stackTop;
    hk_value_decr_ref(val2);
    return;
  }
  HkArray *result = hk_array_add_element(arr, val2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr);
  hk_value_decr_ref(val2);
}

static inline void do_inplace_put_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 2];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  HkValue val3 = slots[2];
  if (!hk_is_array(val1))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  if (!hk_is_int(val2))
  {
    hk_state_runtime_error(state, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_state_runtime_error(state, "range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return;
  }
  if (arr->refCount == 2)
  {
    hk_array_inplace_set_element(arr, (int) index, val3);
    state->stackTop -= 2;
    hk_value_decr_ref(val3);
    return;
  }
  HkArray *result = hk_array_set_element(arr, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  state->stackTop -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
}

static inline void do_inplace_delete_element(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as an array", hk_type_name(val1.type));
    return;
  }
  if (!hk_is_int(val2))
  {
    hk_state_runtime_error(state, "type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return;
  }
  HkArray *arr = hk_as_array(val1);
  int64_t index = (int64_t) hk_as_number(val2);
  if (index < 0 || index >= arr->length)
  {
    hk_state_runtime_error(state, "range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return;
  }
  if (arr->refCount == 2)
  {
    hk_array_inplace_delete_element(arr, (int) index);
    --state->stackTop;
    return;
  }
  HkArray *result = hk_array_delete_element(arr, index);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr);
}

static inline void do_get_field(HkState *state, HkString *name)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_instance(val))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as an instance of struct",
      hk_type_name(val.type));
    return;
  }
  HkInstance *inst = hk_as_instance(val);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_state_runtime_error(state, "no field %.*s on struct", name->length, name->chars);
    return;
  }
  HkValue value = hk_instance_get_field(inst, index);
  hk_value_incr_ref(value);
  slots[0] = value;
  hk_instance_release(inst);
}

static inline void do_fetch_field(HkState *state, HkString *name)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_instance(val))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as an instance of struct",
      hk_type_name(val.type));
    return;
  }
  HkInstance *inst = hk_as_instance(val);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_state_runtime_error(state, "no field %.*s on struct", name->length, name->chars);
    return;
  }
  push(state, hk_number_value(index));
  hk_return_if_not_ok(state);
  HkValue value = hk_instance_get_field(inst, index);
  push(state, value);
  hk_return_if_not_ok(state);
  hk_value_incr_ref(value);
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

static inline void do_put_field(HkState *state, HkString *name)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_instance(val1))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as an instance of struct",
      hk_type_name(val1.type));
    return;
  }
  HkInstance *inst = hk_as_instance(val1);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_state_runtime_error(state, "no field %.*s on struct", name->length, name->chars);
    return;
  }
  HkInstance *result = hk_instance_set_field(inst, index, val2);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  --state->stackTop;
  hk_instance_release(inst);
  hk_value_decr_ref(val2);
}

static inline void do_inplace_put_field(HkState *state, HkString *name)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_instance(val1))
  {
    hk_state_runtime_error(state, "type error: cannot use %s as an instance of struct",
      hk_type_name(val1.type));
    return;
  }
  HkInstance *inst = hk_as_instance(val1);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_state_runtime_error(state, "no field %.*s on struct", name->length, name->chars);
    return;
  }
  if (inst->refCount == 2)
  {
    hk_instance_inplace_set_field(inst, index, val2);
    --state->stackTop;
    hk_value_decr_ref(val2);
    return;
  }
  HkInstance *result = hk_instance_set_field(inst, index, val2);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  --state->stackTop;
  hk_instance_release(inst);
  hk_value_decr_ref(val2);
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

static inline void do_greater(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  hk_state_compare(state, val1, val2, &result);
  hk_return_if_not_ok(state);
  slots[0] = result > 0 ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --state->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline void do_less(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  hk_state_compare(state, val1, val2, &result);
  hk_return_if_not_ok(state);
  slots[0] = result < 0 ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --state->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
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

static inline void do_not_greater(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  hk_state_compare(state, val1, val2, &result);
  hk_return_if_not_ok(state);
  slots[0] = result > 0 ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --state->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline void do_not_less(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  int result;
  hk_state_compare(state, val1, val2, &result);
  hk_return_if_not_ok(state);
  slots[0] = result < 0 ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --state->stackTop;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline void do_bitwise_or(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_state_runtime_error(state, "type error: cannot apply `bitwise or` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  int64_t data = ((int64_t) hk_as_number(val1)) | ((int64_t) hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
}

static inline void do_bitwise_xor(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_state_runtime_error(state, "type error: cannot apply `bitwise xor` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  int64_t data = ((int64_t) hk_as_number(val1)) ^ ((int64_t) hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
}

static inline void do_bitwise_and(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_state_runtime_error(state, "type error: cannot apply `bitwise and` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  int64_t data = ((int64_t) hk_as_number(val1)) & ((int64_t) hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
}

static inline void do_left_shift(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_state_runtime_error(state, "type error: cannot apply `left shift` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  int64_t data = ((int64_t) hk_as_number(val1)) << ((int64_t) hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
}

static inline void do_right_shift(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_state_runtime_error(state, "type error: cannot apply `right shift` between %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  int64_t data = ((int64_t) hk_as_number(val1)) >> ((int64_t) hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
}

static inline void do_add(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (hk_is_number(val1))
  {
    if (!hk_is_number(val2))
    {
      hk_state_runtime_error(state, "type error: cannot add %s to number", hk_type_name(val2.type));
      return;
    }
    double data = hk_as_number(val1) + hk_as_number(val2);
    slots[0] = hk_number_value(data);
    --state->stackTop;
    return;
  }
  if (hk_is_string(val1))
  {
    if (!hk_is_string(val2))
    {
      hk_state_runtime_error(state, "type error: cannot concatenate string and %s",
        hk_type_name(val2.type));
      return;
    }
    concat_strings(state, slots, val1, val2);
    return;
  }
  if (hk_is_array(val1))
  {
    if (!hk_is_array(val2))
    {
      hk_state_runtime_error(state, "type error: cannot concatenate array and %s",
        hk_type_name(val2.type));
      return;
    }
    concat_arrays(state, slots, val1, val2);
    return;
  }
  hk_state_runtime_error(state, "type error: cannot add %s to %s", hk_type_name(val2.type),
    hk_type_name(val1.type));
}

static inline void concat_strings(HkState *state, HkValue *slots, HkValue val1, HkValue val2)
{
  HkString *str1 = hk_as_string(val1);
  if (!str1->length)
  {
    slots[0] = val2;
    --state->stackTop;
    hk_string_release(str1);
    return;
  }
  HkString *str2 = hk_as_string(val2);
  if (!str2->length)
  {
    --state->stackTop;
    hk_string_release(str2);
    return;
  }
  if (str1->refCount == 1)
  {
    hk_string_inplace_concat(str1, str2);
    --state->stackTop;
    hk_string_release(str2);
    return;
  }
  HkString *result = hk_string_concat(str1, str2);
  hk_incr_ref(result);
  slots[0] = hk_string_value(result);
  --state->stackTop;
  hk_string_release(str1);
  hk_string_release(str2);
}

static inline void concat_arrays(HkState *state, HkValue *slots, HkValue val1, HkValue val2)
{
  HkArray *arr1 = hk_as_array(val1);
  if (!arr1->length)
  {
    slots[0] = val2;
    --state->stackTop;
    hk_array_release(arr1);
    return;
  }
  HkArray *arr2 = hk_as_array(val2);
  if (!arr2->length)
  {
    --state->stackTop;
    hk_array_release(arr2);
    return;
  }
  if (arr1->refCount == 1)
  {
    hk_array_inplace_concat(arr1, arr2);
    --state->stackTop;
    hk_array_release(arr2);
    return;
  }
  HkArray *result = hk_array_concat(arr1, arr2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr1);
  hk_array_release(arr2);
}

static inline void do_subtract(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (hk_is_number(val1))
  {
    if (!hk_is_number(val2))
    {
      hk_state_runtime_error(state, "type error: cannot subtract %s from number",
        hk_type_name(val2.type));
      return;
    }
    double data = hk_as_number(val1) - hk_as_number(val2);
    slots[0] = hk_number_value(data);
    --state->stackTop;
    return;
  }
  if (hk_is_array(val1))
  {
    if (!hk_is_array(val2))
    {
      hk_state_runtime_error(state, "type error: cannot diff between array and %s",
        hk_type_name(val2.type));
      return;
    }
    diff_arrays(state, slots, val1, val2);
    return;
  }
  hk_state_runtime_error(state, "type error: cannot subtract %s from %s", hk_type_name(val2.type),
    hk_type_name(val1.type));
}

static inline void diff_arrays(HkState *state, HkValue *slots, HkValue val1, HkValue val2)
{
  HkArray *arr1 = hk_as_array(val1);
  HkArray *arr2 = hk_as_array(val2);
  if (!arr1->length || !arr2->length)
  {
    --state->stackTop;
    hk_array_release(arr2);
    return;
  }
  if (arr1->refCount == 1)
  {
    hk_array_inplace_diff(arr1, arr2);
    --state->stackTop;
    hk_array_release(arr2);
    return;
  }
  HkArray *result = hk_array_diff(arr1, arr2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --state->stackTop;
  hk_array_release(arr1);
  hk_array_release(arr2);
}

static inline void do_multiply(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_state_runtime_error(state, "type error: cannot multiply %s to %s", hk_type_name(val2.type),
      hk_type_name(val1.type));
    return;
  }
  double data = hk_as_number(val1) * hk_as_number(val2);
  slots[0] = hk_number_value(data);
  --state->stackTop;
}

static inline void do_divide(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_state_runtime_error(state, "type error: cannot divide %s by %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  double data = hk_as_number(val1) / hk_as_number(val2);
  slots[0] = hk_number_value(data);
  --state->stackTop;
}

static inline void do_quotient(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_state_runtime_error(state, "type error: cannot apply `quotient` between %s and %s",
      hk_type_name(val1.type), hk_type_name(val2.type));
    return;
  }
  double data = floor(hk_as_number(val1) / hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
}

static inline void do_remainder(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop - 1];
  HkValue val1 = slots[0];
  HkValue val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_state_runtime_error(state, "type error: cannot apply `remainder` between %s and %s",
      hk_type_name(val1.type), hk_type_name(val2.type));
    return;
  }
  double data = fmod(hk_as_number(val1), hk_as_number(val2));
  slots[0] = hk_number_value(data);
  --state->stackTop;
}

static inline void do_negate(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_state_runtime_error(state, "type error: cannot apply `negate` to %s", hk_type_name(val.type));
    return;
  }
  double data = -hk_as_number(val);
  slots[0] = hk_number_value(data);
}

static inline void do_not(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  slots[0] = hk_is_falsey(val) ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  hk_value_release(val);
}

static inline void do_bitwise_not(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_state_runtime_error(state, "type error: cannot apply `bitwise not` to %s", hk_type_name(val.type));
    return;
  }
  int64_t data = ~((int64_t) hk_as_number(val));
  slots[0] = hk_number_value(data);
}

static inline void do_increment(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_state_runtime_error(state, "type error: cannot increment value of type %s",
      hk_type_name(val.type));
    return;
  }
  ++slots[0].as.number;
}

static inline void do_decrement(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  if (!hk_is_number(val))
  {
    hk_state_runtime_error(state, "type error: cannot decrement value of type %s",
      hk_type_name(val.type));
    return;
  }
  --slots[0].as.number;
}

static inline void do_call(HkState *state, int num_args)
{
  HkValue *slots = &state->stackSlots[state->stackTop - num_args];
  HkValue val = slots[0];
  if (!hk_is_callable(val))
  {
    hk_state_runtime_error(state, "type error: cannot call value of type %s",
      hk_type_name(val.type));
    discard_frame(state, slots);
    return;
  }
  if (hk_is_native(val))
  {
    HkNative *native = hk_as_native(val);
    adjust_call_args(state, native->arity, num_args);
    if (!hk_state_is_ok(state))
    {
      discard_frame(state, slots);
      return;
    }
    native->call(state, slots);
    HkSateStatus status = state->status;
    if (status != HK_STATE_STATUS_OK)
    {
      if (hk_state_is_no_trace(state))
      {
        if (hk_state_is_error(state))
          state->flags = HK_STATE_FLAG_NONE;
      }
      else
        print_trace(native->name, NULL, 0);
      if (status == HK_STATE_STATUS_ERROR)
      {
        discard_frame(state, slots);
        return;
      }
      hk_assert(status == HK_STATE_STATUS_EXIT, "status should be exit");
    }
    hk_native_release(native);
    move_result(state, slots);
    return;
  }
  HkClosure *cl = hk_as_closure(val);
  HkFunction *fn = cl->fn;
  adjust_call_args(state, fn->arity, num_args);
  if (!hk_state_is_ok(state))
  {
    discard_frame(state, slots);
    return;
  }
  int line;
  call_function(state, slots, cl, &line);
  HkSateStatus status = state->status;
  if (status != HK_STATE_STATUS_OK)
  {
    if (hk_state_is_no_trace(state))
    {
      if (hk_state_is_error(state))
        state->flags = HK_STATE_FLAG_NONE;
    }
    else
      print_trace(fn->name, fn->file, line);
    if (status == HK_STATE_STATUS_ERROR)
    {
      discard_frame(state, slots);
      return;
    }
    hk_assert(status == HK_STATE_STATUS_EXIT, "status should be exit");
  }
  hk_closure_release(cl);
  move_result(state, slots);
}

static inline void adjust_call_args(HkState *state, int arity, int num_args)
{
  if (num_args > arity)
  {
    do
    {
      pop(state);
      --num_args;
    }
    while (num_args > arity);
    return;
  }
  while (num_args < arity)
  {
    push(state, HK_NIL_VALUE);
    hk_return_if_not_ok(state);
    ++num_args;
  }
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

static inline void call_function(HkState *state, HkValue *locals, HkClosure *cl, int *line)
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
      push(state, HK_NIL_VALUE);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_FALSE:
      push(state, HK_FALSE_VALUE);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_TRUE:
      push(state, HK_TRUE_VALUE);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_INT:
      push(state, hk_number_value(read_word(&pc)));
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_CONSTANT:
      {
        HkValue val = consts[read_byte(&pc)];
        push(state, val);
        if (!hk_state_is_ok(state))
          goto end;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_RANGE:
      do_range(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_ARRAY:
      do_array(state, read_byte(&pc));
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_STRUCT:
      do_struct(state, read_byte(&pc));
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_INSTANCE:
      do_instance(state, read_byte(&pc));
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_CONSTRUCT:
      do_construct(state, read_byte(&pc));
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_ITERATOR:
      do_iterator(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_CLOSURE:
      do_closure(state, functions[read_byte(&pc)]);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_UNPACK_ARRAY:
      do_unpack_array(state, read_byte(&pc));
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_UNPACK_STRUCT:
      do_unpack_struct(state, read_byte(&pc));
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_POP:
      hk_value_release(slots[state->stackTop--]);
      break;
    case HK_OP_GLOBAL:
      {
        HkValue val = slots[read_byte(&pc)];
        push(state, val);
        if (!hk_state_is_ok(state))
          goto end;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_NONLOCAL:
      {
        HkValue val = nonlocals[read_byte(&pc)];
        push(state, val);
        if (!hk_state_is_ok(state))
          goto end;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_LOAD:
      {
        HkValue val = locals[read_byte(&pc)];
        push(state, val);
        if (!hk_state_is_ok(state))
          goto end;
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
      do_add_element(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_GET_ELEMENT:
      do_get_element(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_FETCH_ELEMENT:
      do_fetch_element(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_SET_ELEMENT:
      do_set_element(state);
      break;
    case HK_OP_PUT_ELEMENT:
      do_put_element(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_DELETE_ELEMENT:
      do_delete_element(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_INPLACE_ADD_ELEMENT:
      do_inplace_add_element(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_INPLACE_PUT_ELEMENT:
      do_inplace_put_element(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_INPLACE_DELETE_ELEMENT:
      do_inplace_delete_element(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_GET_FIELD:
      do_get_field(state, hk_as_string(consts[read_byte(&pc)]));
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_FETCH_FIELD:
      do_fetch_field(state, hk_as_string(consts[read_byte(&pc)]));
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_SET_FIELD:
      do_set_field(state);
      break;
    case HK_OP_PUT_FIELD:
      do_put_field(state, hk_as_string(consts[read_byte(&pc)]));
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_INPLACE_PUT_FIELD:
      do_inplace_put_field(state, hk_as_string(consts[read_byte(&pc)]));
      if (!hk_state_is_ok(state))
        goto end;
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
      do_greater(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_LESS:
      do_less(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_NOT_EQUAL:
      do_not_equal(state);
      break;
    case HK_OP_NOT_GREATER:
      do_not_greater(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_NOT_LESS:
      do_not_less(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_BITWISE_OR:
      do_bitwise_or(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_BITWISE_XOR:
      do_bitwise_xor(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_BITWISE_AND:
      do_bitwise_and(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_LEFT_SHIFT:
      do_left_shift(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_RIGHT_SHIFT:
      do_right_shift(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_ADD:
      do_add(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_SUBTRACT:
      do_subtract(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_MULTIPLY:
      do_multiply(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_DIVIDE:
      do_divide(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_QUOTIENT:
      do_quotient(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_REMAINDER:
      do_remainder(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_NEGATE:
      do_negate(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_NOT:
      do_not(state);
      break;
    case HK_OP_BITWISE_NOT:
      do_bitwise_not(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_INCREMENT:
      do_increment(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_DECREMENT:
      do_decrement(state);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_CALL:
      do_call(state, read_byte(&pc));
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_LOAD_MODULE:
      module_load(state, fn->file);
      if (!hk_state_is_ok(state))
        goto end;
      break;
    case HK_OP_RETURN:
      return;
    case HK_OP_RETURN_NIL:
      push(state, HK_NIL_VALUE);
      if (!hk_state_is_ok(state))
        goto end;
      return;
    }
  }
end:
  *line = hk_chunk_get_line(chunk, (int) (pc - code));
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
  state->flags = HK_STATE_FLAG_NONE;
  state->status = HK_STATE_STATUS_OK;
  load_globals(state);
  hk_assert(hk_state_is_ok(state), "state should be ok");
  module_cache_init();
}

void hk_state_deinit(HkState *state)
{
  module_cache_deinit();
  hk_assert(state->stackTop == num_globals() - 1, "stack must contain the globals");
  while (state->stackTop > -1)
    hk_value_release(state->stackSlots[state->stackTop--]);
  free(state->stackSlots);
}

void hk_state_runtime_error(HkState *state, const char *fmt, ...)
{
  state->status = HK_STATE_STATUS_ERROR;
  fprintf(stderr, "runtime error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

void hk_state_check_argument_type(HkState *state, HkValue *args, int index, HkType type)
{
  HkType valType = args[index].type;
  if (valType != type)
    hk_state_runtime_error(state, "type error: argument #%d must be of the type %s, %s given", index,
      hk_type_name(type), hk_type_name(valType));
}

void hk_state_check_argument_types(HkState *state, HkValue *args, int index, int numTypes, HkType types[])
{
  HkType valType = args[index].type;
  bool match = false;
  for (int i = 0; i < numTypes; ++i)
    if ((match = (valType == types[i])))
      break;
  if (!match)
    type_error(state, index, numTypes, types, valType);
}

void hk_state_check_argument_bool(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_BOOL);
}

void hk_state_check_argument_number(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_NUMBER);
}

void hk_state_check_argument_int(HkState *state, HkValue *args, int index)
{
  HkValue val = args[index];
  if (!hk_is_int(val))
    hk_state_runtime_error(state, "type error: argument #%d must be of the type int, %s given",
      index, hk_type_name(val.type));
}

void hk_state_check_argument_string(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_STRING);
}

void hk_state_check_argument_range(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_RANGE);
}

void hk_state_check_argument_array(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_ARRAY);
}

void hk_state_check_argument_struct(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_STRUCT);
}

void hk_state_check_argument_instance(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_INSTANCE);
}

void hk_state_check_argument_iterator(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_ITERATOR);
}

void hk_state_check_argument_callable(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_CALLABLE);
}

void hk_state_check_argument_userdata(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_USERDATA);
}

void hk_state_push(HkState *state, HkValue val)
{
  push(state, val);
  hk_return_if_not_ok(state);
  hk_value_incr_ref(val);
}

void hk_state_push_nil(HkState *state)
{
  push(state, HK_NIL_VALUE);
}

void hk_state_push_bool(HkState *state, bool data)
{
  push(state, data ? HK_TRUE_VALUE : HK_FALSE_VALUE);
}

void hk_state_push_number(HkState *state, double data)
{
  push(state, hk_number_value(data));
}

void hk_state_push_string(HkState *state, HkString *str)
{
  push(state, hk_string_value(str));
  hk_return_if_not_ok(state);
  hk_incr_ref(str);
}

void hk_state_push_string_from_chars(HkState *state, int length, const char *chars)
{
  HkString *str = hk_string_from_chars(length, chars);
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

void hk_state_push_string_from_stream(HkState *state, FILE *stream, const char delim)
{
  HkString *str = hk_string_from_stream(stream, delim);
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

void hk_state_push_range(HkState *state, HkRange *range)
{
  push(state, hk_range_value(range));
  hk_return_if_not_ok(state);
  hk_incr_ref(range);
}

void hk_state_push_array(HkState *state, HkArray *arr)
{
  push(state, hk_array_value(arr));
  hk_return_if_not_ok(state);
  hk_incr_ref(arr);
}

void hk_state_push_struct(HkState *state, HkStruct *ztruct)
{
  push(state, hk_struct_value(ztruct));
  hk_return_if_not_ok(state);
  hk_incr_ref(ztruct);
}

void hk_state_push_instance(HkState *state, HkInstance *inst)
{
  push(state, hk_instance_value(inst));
  hk_return_if_not_ok(state);
  hk_incr_ref(inst);
}

void hk_state_push_iterator(HkState *state, HkIterator *it)
{
  push(state, hk_iterator_value(it));
  hk_return_if_not_ok(state);
  hk_incr_ref(it);
}

void hk_state_push_closure(HkState *state, HkClosure *cl)
{
  push(state, hk_closure_value(cl));
  hk_return_if_not_ok(state);
  hk_incr_ref(cl);
}

void hk_state_push_native(HkState *state, HkNative *native)
{
  push(state, hk_native_value(native));
  hk_return_if_not_ok(state);
  hk_incr_ref(native);
}

void hk_state_push_new_native(HkState *state, const char *name, int arity, void (*call)(HkState *, HkValue *))
{
  HkNative *native = hk_native_new(hk_string_from_chars(-1, name), arity, call);
  hk_state_push_native(state, native);
  if (!hk_state_is_ok(state))
    hk_native_free(native);
}

void hk_state_push_userdata(HkState *state, HkUserdata *udata)
{
  push(state, hk_userdata_value(udata));
  hk_return_if_not_ok(state);
  hk_incr_ref(udata);
}

void hk_state_array(HkState *state, int length)
{
  do_array(state, length);
}

void hk_state_struct(HkState *state, int length)
{
  do_struct(state, length);
}

void hk_state_instance(HkState *state, int num_args)
{
  do_instance(state, num_args);
}

void hk_state_construct(HkState *state, int length)
{
  do_construct(state, length);
}

void hk_state_pop(HkState *state)
{
  pop(state);
}

void hk_state_call(HkState *state, int num_args)
{
  do_call(state, num_args);
}

void hk_state_compare(HkState *state, HkValue val1, HkValue val2, int *result)
{
  if (!hk_is_comparable(val1))
  {
    hk_state_runtime_error(state, "type error: value of type %s is not comparable", hk_type_name(val1.type));
    return;
  }
  if (val1.type != val2.type)
  {
    hk_state_runtime_error(state, "type error: cannot compare %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return;
  }
  hk_assert(hk_value_compare(val1, val2, result), "hk_value_compare failed");
}
