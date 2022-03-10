//
// Hook Programming Language
// hook_arrays.c
//

#include "hook_arrays.h"
#include <stdlib.h>

static int new_array_call(vm_t *vm, value_t *args);
static int index_of_call(vm_t *vm, value_t *args);
static int min_call(vm_t *vm, value_t *args);
static int max_call(vm_t *vm, value_t *args);
static int sum_call(vm_t *vm, value_t *args);

static int new_array_call(vm_t *vm, value_t *args)
{
  if (vm_check_int(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  int capacity = (int) args[1].as.number;
  array_t *arr = array_allocate(capacity);
  arr->length = 0;
  if (vm_push_array(vm, arr) == STATUS_ERROR)
  {
    array_free(arr);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int index_of_call(vm_t *vm, value_t *args)
{
  if (vm_check_array(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, array_index_of(AS_ARRAY(args[1]), args[2]));
}

static int min_call(vm_t *vm, value_t *args)
{
  if (vm_check_array(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  array_t *arr = AS_ARRAY(args[1]);
  int length = arr->length;
  if (!length)
    return vm_push_nil(vm);
  value_t min = arr->elements[0];
  for (int i = 1; i < length; ++i)
  {
    value_t elem = arr->elements[i];
    int result;
    if (value_compare(elem, min, &result) == STATUS_ERROR)
      return STATUS_ERROR;
    min = result < 0 ? elem : min;
  }
  return vm_push(vm, min);
}

static int max_call(vm_t *vm, value_t *args)
{
  if (vm_check_array(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  array_t *arr = AS_ARRAY(args[1]);
  int length = arr->length;
  if (!length)
    return vm_push_nil(vm);
  value_t max = arr->elements[0];
  for (int i = 1; i < length; ++i)
  {
    value_t elem = arr->elements[i];
    int result;
    if (value_compare(elem, max, &result) == STATUS_ERROR)
      return STATUS_ERROR;
    max = result > 0 ? elem : max;
  }
  return vm_push(vm, max);
}

static int sum_call(vm_t *vm, value_t *args)
{
  if (vm_check_array(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  array_t *arr = AS_ARRAY(args[1]);
  double sum = 0;
  for (int i = 0; i < arr->length; ++i)
  {
    value_t elem = arr->elements[i];
    if (!IS_NUMBER(elem))
    {
      sum = 0;
      break;
    }
    sum += elem.as.number;
  }
  vm_push_number(vm, sum);
  return STATUS_OK;
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_arrays(vm_t *vm)
#else
int load_arrays(vm_t *vm)
#endif
{
  if (vm_push_string_from_chars(vm,-1, "arrays") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm,-1, "new_array") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "new_array", 1, &new_array_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm,-1, "index_of") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "index_of", 2, &index_of_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm,-1, "min") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "min", 1, &min_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm,-1, "max") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "max", 1, &max_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm,-1, "sum") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "sum", 1, &sum_call) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_construct(vm, 5);
}
