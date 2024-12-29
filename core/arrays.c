//
// array.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "arrays.h"

static void new_array_call(HkVM *vm, HkValue *args);
static void fill_call(HkVM *vm, HkValue *args);
static void index_of_call(HkVM *vm, HkValue *args);
static void min_call(HkVM *vm, HkValue *args);
static void max_call(HkVM *vm, HkValue *args);
static void sum_call(HkVM *vm, HkValue *args);
static void avg_call(HkVM *vm, HkValue *args);
static void reverse_call(HkVM *vm, HkValue *args);
static void sort_call(HkVM *vm, HkValue *args);

static void new_array_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int capacity = (int) hk_as_number(args[1]);
  HkArray *arr = hk_array_new_with_capacity(capacity);
  hk_vm_push_array(vm, arr);
  if (!hk_vm_is_ok(vm))
    hk_array_free(arr);
}

static void fill_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
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
  hk_vm_push_array(vm, arr);
  if (!hk_vm_is_ok(vm))
    hk_array_free(arr);
}

static void index_of_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkArray *arr = hk_as_array(args[1]);
  HkValue elem = args[2];
  int index = hk_array_index_of(arr, elem);
  hk_vm_push_number(vm, index);
}

static void min_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkArray *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
  {
    hk_vm_push_nil(vm);
    return;
  }
  HkValue min = hk_array_get_element(arr, 0);
  for (int i = 1; i < length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    int result;
    hk_vm_compare(vm, elem, min, &result);
    hk_return_if_not_ok(vm);
    min = result < 0 ? elem : min;
  }
  hk_vm_push(vm, min);
}

static void max_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkArray *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
  {
    hk_vm_push_nil(vm);
    return;
  }
  HkValue max = hk_array_get_element(arr, 0);
  for (int i = 1; i < length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    int result;
    hk_vm_compare(vm, elem, max, &result);
    hk_return_if_not_ok(vm);
    max = result > 0 ? elem : max;
  }
  hk_vm_push(vm, max);
}

static void sum_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
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
  hk_vm_push_number(vm, sum);
}

static void avg_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkArray *arr = hk_as_array(args[1]);
  int length = arr->length;
  if (!length)
  {
    hk_vm_push_number(vm, 0);
    return;
  }
  double sum = 0;
  for (int i = 0; i < length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    if (!hk_is_number(elem))
    {
      hk_vm_push_number(vm, 0);
      return;
    }
    sum += hk_as_number(elem);
  }
  hk_vm_push_number(vm, sum / length);
}

static void reverse_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkArray *arr = hk_array_reverse(hk_as_array(args[1]));
  hk_vm_push_array(vm, arr);
  if (!hk_vm_is_ok(vm))
    hk_array_free(arr);
}

static void sort_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_array(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkArray *arr;
  if (!hk_array_sort(hk_as_array(args[1]), &arr))
  {
    hk_vm_runtime_error(vm, "cannot compare elements of array");
    return;
  }
  hk_vm_push_array(vm, arr);
  if (!hk_vm_is_ok(vm))
    hk_array_free(arr);
}

HK_LOAD_MODULE_HANDLER(arrays)
{
  hk_vm_push_string_from_chars(vm, -1, "arrays");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "new_array");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "new_array", 1, new_array_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "fill");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "fill", 2, fill_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "index_of");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "index_of", 2, index_of_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "min");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "min", 1, min_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "max");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "max", 1, max_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "sum");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "sum", 1, sum_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "avg");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "avg", 1, avg_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "reverse");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "reverse", 1, reverse_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "sort");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "sort", 1, sort_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 9);
}
