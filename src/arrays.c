//
// Hook Programming Language
// arrays.c
//

#include "arrays.h"
#include <stdlib.h>
#include <limits.h>
#include "common.h"
#include "error.h"

static int new_array_call(vm_t *vm, value_t *args);
static int index_of_call(vm_t *vm, value_t *args);
static int min_call(vm_t *vm, value_t *args);
static int max_call(vm_t *vm, value_t *args);
static int sum_call(vm_t *vm, value_t *args);

static int new_array_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_INTEGER(val))
  {
    runtime_error("type error: expected integer but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  long capacity = (long) val.as.number;
  if (capacity < 0 || capacity > INT_MAX)
  {
    runtime_error("invalid range: capacity must be between 0 and %d", INT_MAX);
    return STATUS_ERROR;
  }
  array_t *arr = array_allocate((int) capacity);
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
  value_t val1 = args[1];
  value_t val2 = args[2];
  if (!IS_ARRAY(val1))
  {
    runtime_error("type error: expected array but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, array_index_of(AS_ARRAY(val1), val2));
}

static int min_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_ARRAY(val))
  {
    runtime_error("type error: expected array but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val);
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
  value_t val = args[1];
  if (!IS_ARRAY(val))
  {
    runtime_error("type error: expected array but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val);
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
  value_t val = args[1];
  if (!IS_ARRAY(val))
  {
    runtime_error("type error: expected array but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val);
  double sum = 0;
  for (int i = 0; i < arr->length; ++i)
  {
    value_t elem = arr->elements[i];
    if (!IS_NUMBER(elem))
    {
      runtime_error("type error: expected array of numbers, found `%s` in array", type_name(elem.type));
      return STATUS_ERROR;
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
