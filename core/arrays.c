//
// The Hook Programming Language
// arrays.c
//

#include "arrays.h"

static void new_array_call(HkState *state, HkValue *args);
static void fill_call(HkState *state, HkValue *args);
static void index_of_call(HkState *state, HkValue *args);
static void min_call(HkState *state, HkValue *args);
static void max_call(HkState *state, HkValue *args);
static void sum_call(HkState *state, HkValue *args);
static void avg_call(HkState *state, HkValue *args);
static void reverse_call(HkState *state, HkValue *args);
static void sort_call(HkState *state, HkValue *args);

static void new_array_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_int(state, args, 1);
  hk_return_if_not_ok(state);
  int capacity = (int) hk_as_number(args[1]);
  HkArray *arr = hk_array_new_with_capacity(capacity);
  hk_state_push_array(state, arr);
  if (!hk_state_is_ok(state))
    hk_array_free(arr);
}

static void fill_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  HkValue elem = args[1];
  int count = (int) hk_as_number(args[2]);
  count = count < 0 ? 0 : count;
  HkArray *arr = hk_array_new_with_capacity(count);
  for (int i = 0; i < count; ++i)
  {
    hk_value_incr_ref(elem);
    arr->elements[i] = elem;
  }
  arr->length = count;
  hk_state_push_array(state, arr);
  if (!hk_state_is_ok(state))
    hk_array_free(arr);
}

static void index_of_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_array(state, args, 1);
  hk_return_if_not_ok(state);
  HkArray *arr = hk_as_array(args[1]);
  HkValue elem = args[2];
  int index = hk_array_index_of(arr, elem);
  hk_state_push_number(state, index);
}

static void min_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_array(state, args, 1);
  hk_return_if_not_ok(state);
  HkArray *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
  {
    hk_state_push_nil(state);
    return;
  }
  HkValue min = hk_array_get_element(arr, 0);
  for (int i = 1; i < length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    int result;
    hk_state_compare(state, elem, min, &result);
    hk_return_if_not_ok(state);
    min = result < 0 ? elem : min;
  }
  hk_state_push(state, min);
}

static void max_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_array(state, args, 1);
  hk_return_if_not_ok(state);
  HkArray *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
  {
    hk_state_push_nil(state);
    return;
  }
  HkValue max = hk_array_get_element(arr, 0);
  for (int i = 1; i < length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    int result;
    hk_state_compare(state, elem, max, &result);
    hk_return_if_not_ok(state);
    max = result > 0 ? elem : max;
  }
  hk_state_push(state, max);
}

static void sum_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_array(state, args, 1);
  hk_return_if_not_ok(state);
  HkArray *arr = hk_as_array(args[1]);
  double sum = 0;
  for (int i = 0; i < arr->length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    if (!hk_is_number(elem))
    {
      sum = 0;
      break;
    }
    sum += hk_as_number(elem);
  }
  hk_state_push_number(state, sum);
}

static void avg_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_array(state, args, 1);
  hk_return_if_not_ok(state);
  HkArray *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
  {
    hk_state_push_number(state, 0);
    return;
  }
  double sum = 0;
  for (int i = 0; i < length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    if (!hk_is_number(elem))
    {
      hk_state_push_number(state, 0);
      return;
    }
    sum += hk_as_number(elem);
  }
  hk_state_push_number(state, sum / length);
}

static void reverse_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_array(state, args, 1);
  hk_return_if_not_ok(state);
  HkArray *arr = hk_array_reverse(hk_as_array(args[1]));
  hk_state_push_array(state, arr);
  if (!hk_state_is_ok(state))
    hk_array_free(arr);
}

static void sort_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_array(state, args, 1);
  hk_return_if_not_ok(state);
  HkArray *arr;
  if (!hk_array_sort(hk_as_array(args[1]), &arr))
  {
    hk_state_runtime_error(state, "cannot compare elements of array");
    return;
  }
  hk_state_push_array(state, arr);
  if (!hk_state_is_ok(state))
    hk_array_free(arr);
}

HK_LOAD_MODULE_HANDLER(arrays)
{
  hk_state_push_string_from_chars(state, -1, "arrays");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new_array");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new_array", 1, new_array_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "fill");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "fill", 2, fill_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "index_of");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "index_of", 2, index_of_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "min");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "min", 1, min_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "max");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "max", 1, max_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sum");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sum", 1, sum_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "avg");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "avg", 1, avg_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "reverse");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "reverse", 1, reverse_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sort");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sort", 1, sort_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 9);
}
