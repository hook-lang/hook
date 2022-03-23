//
// Hook Programming Language
// hook_arrays.c
//

#include "hook_arrays.h"
#include <stdlib.h>

static int new_array_call(hk_vm_t *vm, hk_value_t *args);
static int index_of_call(hk_vm_t *vm, hk_value_t *args);
static int min_call(hk_vm_t *vm, hk_value_t *args);
static int max_call(hk_vm_t *vm, hk_value_t *args);
static int sum_call(hk_vm_t *vm, hk_value_t *args);

static int new_array_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_int(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  int capacity = (int) args[1].as.number;
  hk_array_t *arr = hk_array_new_with_capacity(capacity);
  if (hk_vm_push_array(vm, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int index_of_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, hk_array_index_of(hk_as_array(args[1]), args[2]));
}

static int min_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_array_t *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
    return hk_vm_push_nil(vm);
  hk_value_t min = hk_array_get_element(arr, 0);
  for (int i = 1; i < length; ++i)
  {
    hk_value_t elem = hk_array_get_element(arr, i);
    int result;
    if (hk_value_compare(elem, min, &result) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    min = result < 0 ? elem : min;
  }
  return hk_vm_push(vm, min);
}

static int max_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_array_t *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
    return hk_vm_push_nil(vm);
  hk_value_t max = hk_array_get_element(arr, 0);
  for (int i = 1; i < length; ++i)
  {
    hk_value_t elem = hk_array_get_element(arr, i);
    int result;
    if (hk_value_compare(elem, max, &result) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    max = result > 0 ? elem : max;
  }
  return hk_vm_push(vm, max);
}

static int sum_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_array(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_array_t *arr = hk_as_array(args[1]);
  double sum = 0;
  for (int i = 0; i < arr->length; ++i)
  {
    hk_value_t elem = hk_array_get_element(arr, i);
    if (!hk_is_number(elem))
    {
      sum = 0;
      break;
    }
    sum += elem.as.number;
  }
  hk_vm_push_number(vm, sum);
  return HK_STATUS_OK;
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_arrays(hk_vm_t *vm)
#else
int load_arrays(hk_vm_t *vm)
#endif
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
  return hk_vm_construct(vm, 5);
}
