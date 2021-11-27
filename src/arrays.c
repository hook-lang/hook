//
// Hook Programming Language
// arrays.c
//

#include "arrays.h"
#include <limits.h>
#include "common.h"
#include "error.h"

static int new_array_call(vm_t *vm, value_t *frame);
static int index_of_call(vm_t *vm, value_t *frame);
static int min_call(vm_t *vm, value_t *frame);
static int max_call(vm_t *vm, value_t *frame);
static int sum_call(vm_t *vm, value_t *frame);

static int new_array_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_INTEGER(val))
  {
    runtime_error("invalid type: expected integer but got `%s`", type_name(val.type));
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

static int index_of_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_ARRAY(val1))
  {
    runtime_error("invalid type: expected array but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, array_index_of(AS_ARRAY(val1), val2));
}

static int min_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_ARRAY(val))
  {
    runtime_error("invalid type: expected array but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val);
  int length = arr->length;
  if (!length)
  {
    vm_push_null(vm);
    return STATUS_OK;
  }
  value_t min = arr->elements[0];
  for (int i = 1; i < length; ++i)
  {
    value_t elem = arr->elements[i];
    int result;
    if (value_compare(elem, min, &result) == STATUS_ERROR)
      return STATUS_ERROR;
    min = result < 0 ? elem : min;
  }
  vm_push_value(vm, min);
  return STATUS_OK;
}

static int max_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_ARRAY(val))
  {
    runtime_error("invalid type: expected array but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val);
  int length = arr->length;
  if (!length)
  {
    vm_push_null(vm);
    return STATUS_OK;
  }
  value_t max = arr->elements[0];
  for (int i = 1; i < length; ++i)
  {
    value_t elem = arr->elements[i];
    int result;
    if (value_compare(elem, max, &result) == STATUS_ERROR)
      return STATUS_ERROR;
    max = result > 0 ? elem : max;
  }
  vm_push_value(vm, max);
  return STATUS_OK;
}

static int sum_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_ARRAY(val))
  {
    runtime_error("invalid type: expected array but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val);
  double sum = 0;
  for (int i = 0; i < arr->length; ++i)
  {
    value_t elem = arr->elements[i];
    if (!IS_NUMBER(elem))
    {
      runtime_error("invalid type: expected array of numbers, found `%s` in array", type_name(elem.type));
      return STATUS_ERROR;
    }
    sum += elem.as.number;
  }
  vm_push_number(vm, sum);
  return STATUS_OK;
}

#ifdef _WIN32
void __declspec(dllexport) __stdcall load_arrays(vm_t *vm)
#else
void load_arrays(vm_t *vm)
#endif
{
  char new_array[] = "new_array";
  char index_of[] = "index_of";
  char min[] = "min";
  char max[] = "max";
  char sum[] = "sum";
  struct_t *ztruct = struct_new(string_from_chars(-1, "arrays"));
  struct_put(ztruct, sizeof(new_array) - 1, new_array);
  struct_put(ztruct, sizeof(index_of) - 1, index_of);
  struct_put(ztruct, sizeof(min) - 1, min);
  struct_put(ztruct, sizeof(max) - 1, max);
  struct_put(ztruct, sizeof(sum) - 1, sum);
  vm_push_native(vm, native_new(string_from_chars(-1, new_array), 1, &new_array_call));
  vm_push_native(vm, native_new(string_from_chars(-1, new_array), 2, &index_of_call));
  vm_push_native(vm, native_new(string_from_chars(-1, new_array), 1, &min_call));
  vm_push_native(vm, native_new(string_from_chars(-1, new_array), 1, &max_call));
  vm_push_native(vm, native_new(string_from_chars(-1, new_array), 1, &sum_call));
  vm_push_struct(vm, ztruct);
  vm_instance(vm);
}
