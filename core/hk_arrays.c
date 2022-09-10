//
// The Hook Programming Language
// hk_arrays.c
//

#include "hk_arrays.h"
#include <stdlib.h>
#include "hk_status.h"
#include "hk_error.h"

static int32_t new_array_call(hk_vm_t *vm, hk_value_t *args);
static int32_t index_of_call(hk_vm_t *vm, hk_value_t *args);
static int32_t min_call(hk_vm_t *vm, hk_value_t *args);
static int32_t max_call(hk_vm_t *vm, hk_value_t *args);
static int32_t sum_call(hk_vm_t *vm, hk_value_t *args);
static int32_t avg_call(hk_vm_t *vm, hk_value_t *args);
static int32_t reverse_call(hk_vm_t *vm, hk_value_t *args);
static int32_t sort_call(hk_vm_t *vm, hk_value_t *args);

static int32_t new_array_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_int(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  int32_t capacity = (int32_t) hk_as_float(args[1]);
  hk_array_t *arr = hk_array_new_with_capacity(capacity);
  if (hk_vm_push_array(vm, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t index_of_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, hk_array_index_of(hk_as_array(args[1]), args[2]));
}

static int32_t min_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_array_t *arr = hk_as_array(args[1]);
  int32_t length = arr->length;
  if (!length)
    return hk_vm_push_nil(vm);
  hk_value_t min = hk_array_get_element(arr, 0);
  for (int32_t i = 1; i < length; ++i)
  {
    hk_value_t elem = hk_array_get_element(arr, i);
    int32_t result;
    if (hk_vm_compare(vm, elem, min, &result) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    min = result < 0 ? elem : min;
  }
  return hk_vm_push(vm, min);
}

static int32_t max_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_array_t *arr = hk_as_array(args[1]);
  int32_t length = arr->length;
  if (!length)
    return hk_vm_push_nil(vm);
  hk_value_t max = hk_array_get_element(arr, 0);
  for (int32_t i = 1; i < length; ++i)
  {
    hk_value_t elem = hk_array_get_element(arr, i);
    int32_t result;
    if (hk_vm_compare(vm, elem, max, &result) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    max = result > 0 ? elem : max;
  }
  return hk_vm_push(vm, max);
}

static int32_t sum_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_array_t *arr = hk_as_array(args[1]);
  double sum = 0;
  for (int32_t i = 0; i < arr->length; ++i)
  {
    hk_value_t elem = hk_array_get_element(arr, i);
    if (!hk_is_float(elem))
    {
      sum = 0;
      break;
    }
    sum += hk_as_float(elem);
  }
  return hk_vm_push_float(vm, sum);
}

static int32_t avg_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_array_t *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
    return hk_vm_push_float(vm, 0);
  double sum = 0;
  for (int32_t i = 0; i < length; ++i)
  {
    hk_value_t elem = hk_array_get_element(arr, i);
    if (!hk_is_float(elem))
      return hk_vm_push_float(vm, 0);
    sum += hk_as_float(elem);
  }
  return hk_vm_push_float(vm, sum / length);
}

static int32_t reverse_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_array_t *arr = hk_array_reverse(hk_as_array(args[1]));
  if (hk_vm_push_array(vm, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t sort_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_array_t *arr;
  if (!hk_array_sort(hk_as_array(args[1]), &arr))
  {
    hk_runtime_error("cannot compare elements of array");
    return HK_STATUS_ERROR;
  }
  if (hk_vm_push_array(vm, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

HK_LOAD_FN(arrays)
{
  if (hk_vm_push_string_from_chars(vm,-1, "arrays") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm,-1, "new_array") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "new_array", 1, &new_array_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm,-1, "index_of") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "index_of", 2, &index_of_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm,-1, "min") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "min", 1, &min_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm,-1, "max") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "max", 1, &max_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm,-1, "sum") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sum", 1, &sum_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm,-1, "avg") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "avg", 1, &avg_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm,-1, "reverse") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "reverse", 1, &reverse_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;  
  if (hk_vm_push_string_from_chars(vm,-1, "sort") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sort", 1, &sort_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 8);
}
