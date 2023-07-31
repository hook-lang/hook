//
// The Hook Programming Language
// arrays.c
//

#include "arrays.h"
#include <hook/check.h>
#include <hook/status.h>
#include <hook/error.h>

static int new_array_call(HkState *state, HkValue *args);
static int fill_call(HkState *state, HkValue *args);
static int index_of_call(HkState *state, HkValue *args);
static int min_call(HkState *state, HkValue *args);
static int max_call(HkState *state, HkValue *args);
static int sum_call(HkState *state, HkValue *args);
static int avg_call(HkState *state, HkValue *args);
static int reverse_call(HkState *state, HkValue *args);
static int sort_call(HkState *state, HkValue *args);

static int new_array_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_int(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  int capacity = (int) hk_as_number(args[1]);
  HkArray *arr = hk_array_new_with_capacity(capacity);
  if (hk_state_push_array(state, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int fill_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
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
  if (hk_state_push_array(state, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int index_of_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, hk_array_index_of(hk_as_array(args[1]), args[2]));
}

static int min_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkArray *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
    return hk_state_push_nil(state);
  HkValue min = hk_array_get_element(arr, 0);
  for (int i = 1; i < length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    int result;
    if (hk_state_compare(elem, min, &result) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    min = result < 0 ? elem : min;
  }
  return hk_state_push(state, min);
}

static int max_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkArray *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
    return hk_state_push_nil(state);
  HkValue max = hk_array_get_element(arr, 0);
  for (int i = 1; i < length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    int result;
    if (hk_state_compare(elem, max, &result) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    max = result > 0 ? elem : max;
  }
  return hk_state_push(state, max);
}

static int sum_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
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
  return hk_state_push_number(state, sum);
}

static int avg_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkArray *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
    return hk_state_push_number(state, 0);
  double sum = 0;
  for (int i = 0; i < length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    if (!hk_is_number(elem))
      return hk_state_push_number(state, 0);
    sum += hk_as_number(elem);
  }
  return hk_state_push_number(state, sum / length);
}

static int reverse_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkArray *arr = hk_array_reverse(hk_as_array(args[1]));
  if (hk_state_push_array(state, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int sort_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkArray *arr;
  if (!hk_array_sort(hk_as_array(args[1]), &arr))
  {
    hk_runtime_error("cannot compare elements of array");
    return HK_STATUS_ERROR;
  }
  if (hk_state_push_array(state, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

HK_LOAD_FN(arrays)
{
  if (hk_state_push_string_from_chars(state, -1, "arrays") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "new_array") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "new_array", 1, &new_array_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "fill") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "fill", 2, &fill_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "index_of") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "index_of", 2, &index_of_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "min") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "min", 1, &min_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "max") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "max", 1, &max_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sum") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sum", 1, &sum_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "avg") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "avg", 1, &avg_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "reverse") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "reverse", 1, &reverse_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;  
  if (hk_state_push_string_from_chars(state, -1, "sort") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sort", 1, &sort_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 9);
}
