//
// The Hook Programming Language
// hk_builtin.c
//

#include "hk_builtin.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <ctype.h>
#include <limits.h>

#ifdef _WIN32
  #include <Windows.h>
#endif

#ifndef _WIN32
  #include <unistd.h>
#endif

#include "hk_struct.h"
#include "hk_iterable.h"
#include "hk_status.h"
#include "hk_error.h"
#include "hk_utils.h"

#ifdef _WIN32
  #define strtok_r strtok_s
#endif

static const char *globals[] = {
  "print",
  "println",
  "type",
  "is_nil",
  "is_bool",
  "is_number",
  "is_int",
  "is_string",
  "is_range",
  "is_array",
  "is_struct",
  "is_instance",
  "is_iterator",
  "is_callable",
  "is_userdata",
  "is_object",
  "is_comparable",
  "is_iterable",
  "to_bool",
  "to_int",
  "to_number",
  "to_string",
  "ord",
  "chr",
  "hex",
  "bin",
  "address",
  "refcount",
  "cap",
  "len",
  "is_empty",
  "compare",
  "split",
  "join",
  "iter",
  "valid",
  "current",
  "next",
  "sleep",
  "assert",
  "panic"
};

static inline int32_t string_to_double(hk_vm_t *vm, hk_string_t *str, double *result);
static inline hk_array_t *split(hk_string_t *str, hk_string_t *separator);
static inline int32_t join(hk_array_t *arr, hk_string_t *separator, hk_string_t **result);
static int32_t print_call(hk_vm_t *vm, hk_value_t *args);
static int32_t println_call(hk_vm_t *vm, hk_value_t *args);
static int32_t type_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_nil_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_bool_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_number_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_int_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_string_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_range_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_array_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_struct_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_instance_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_iterator_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_callable_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_userdata_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_object_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_comparable_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_iterable_call(hk_vm_t *vm, hk_value_t *args);
static int32_t to_bool_call(hk_vm_t *vm, hk_value_t *args);
static int32_t to_int_call(hk_vm_t *vm, hk_value_t *args);
static int32_t to_number_call(hk_vm_t *vm, hk_value_t *args);
static int32_t to_string_call(hk_vm_t *vm, hk_value_t *args);
static int32_t ord_call(hk_vm_t *vm, hk_value_t *args);
static int32_t chr_call(hk_vm_t *vm, hk_value_t *args);
static int32_t hex_call(hk_vm_t *vm, hk_value_t *args);
static int32_t bin_call(hk_vm_t *vm, hk_value_t *args);
static int32_t address_call(hk_vm_t *vm, hk_value_t *args);
static int32_t refcount_call(hk_vm_t *vm, hk_value_t *args);
static int32_t cap_call(hk_vm_t *vm, hk_value_t *args);
static int32_t len_call(hk_vm_t *vm, hk_value_t *args);
static int32_t is_empty_call(hk_vm_t *vm, hk_value_t *args);
static int32_t compare_call(hk_vm_t *vm, hk_value_t *args);
static int32_t split_call(hk_vm_t *vm, hk_value_t *args);
static int32_t join_call(hk_vm_t *vm, hk_value_t *args);
static int32_t iter_call(hk_vm_t *vm, hk_value_t *args);
static int32_t valid_call(hk_vm_t *vm, hk_value_t *args);
static int32_t current_call(hk_vm_t *vm, hk_value_t *args);
static int32_t next_call(hk_vm_t *vm, hk_value_t *args);
static int32_t sleep_call(hk_vm_t *vm, hk_value_t *args);
static int32_t assert_call(hk_vm_t *vm, hk_value_t *args);
static int32_t panic_call(hk_vm_t *vm, hk_value_t *args);

static inline int32_t string_to_double(hk_vm_t *vm, hk_string_t *str, double *result)
{
  (void) vm;
  if (!str->length)
  {
    hk_runtime_error("type error: argument #1 must be a non-empty string");
    return HK_STATUS_ERROR;
  }
  if (!hk_double_from_chars(result, str->chars))
  {
    hk_runtime_error("type error: argument #1 is not a convertible string");
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static inline hk_array_t *split(hk_string_t *str, hk_string_t *separator)
{
  hk_array_t *arr = hk_array_new();
  char *cur = str->chars;
  char *tk;
  while ((tk = strtok_r(cur, separator->chars, &cur)))
  {
    hk_value_t elem = hk_string_value(hk_string_from_chars(-1, tk));
    hk_array_inplace_add_element(arr, elem);
  }
  return arr;
}

static inline int32_t join(hk_array_t *arr, hk_string_t *separator, hk_string_t **result)
{
  hk_string_t *str = hk_string_new();
  for (int32_t i = 0; i < arr->length; ++i)
  {
    hk_value_t elem = hk_array_get_element(arr, i);
    if (!hk_is_string(elem))
      continue;
    if (i)
      hk_string_inplace_concat(str, separator);
    hk_string_inplace_concat(str, hk_as_string(elem));
  }
  *result = str;
  return HK_STATUS_OK;
}

static int32_t print_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_value_print(args[1], false);
  return hk_vm_push_nil(vm);
}

static int32_t println_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_value_print(args[1], false);
  printf("\n");
  return hk_vm_push_nil(vm);
}

static int32_t type_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_string_from_chars(vm, -1, hk_type_name(args[1].type));
}

static int32_t is_nil_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_nil(args[1]));
}

static int32_t is_bool_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_bool(args[1]));
}

static int32_t is_number_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_number(args[1]));
}

static int32_t is_int_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_int(args[1]));
}

static int32_t is_string_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_string(args[1]));
}

static int32_t is_range_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_range(args[1]));
}

static int32_t is_array_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_array(args[1]));
}

static int32_t is_struct_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_struct(args[1]));
}

static int32_t is_instance_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_instance(args[1]));
}

static int32_t is_iterator_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_iterator(args[1]));
}

static int32_t is_callable_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_callable(args[1]));
}

static int32_t is_userdata_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_userdata(args[1]));
}

static int32_t is_object_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_object(args[1]));
}

static int32_t is_comparable_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_comparable(args[1]));
}

static int32_t is_iterable_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_iterable(args[1]));
}

static int32_t to_bool_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_bool(vm, hk_is_truthy(args[1]));
}

static int32_t to_int_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_type_t types[] = {HK_TYPE_NUMBER, HK_TYPE_STRING};
  if (hk_vm_check_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  if (hk_is_number(val))
    return hk_vm_push_number(vm, (int64_t) hk_as_number(val));
  double result;
  if (string_to_double(vm, hk_as_string(val), &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, (int64_t) result);
}

static int32_t to_number_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_type_t types[] = {HK_TYPE_NUMBER, HK_TYPE_STRING};
  if (hk_vm_check_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  if (hk_is_number(val))
    return HK_STATUS_OK;
  double result;
  if (string_to_double(vm, hk_as_string(args[1]), &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, result);
}

static int32_t to_string_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_type_t types[] = {HK_TYPE_NIL, HK_TYPE_BOOL, HK_TYPE_NUMBER, HK_TYPE_STRING};
  if (hk_vm_check_types(args, 1, 4, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  hk_string_t *str;
  if (hk_is_nil(val))
  {
    str = hk_string_from_chars(-1, "nil");
    goto end;
  }
  if (hk_is_bool(val))
  {
    str = hk_string_from_chars(-1, hk_as_bool(val) ? "true" : "false");
    goto end;
  }
  if (hk_is_number(val))
  {
    char chars[32];
    snprintf(chars, sizeof(chars) - 1,  "%g", hk_as_number(val));
    str = hk_string_from_chars(-1, chars);
    goto end;
  }
  return hk_vm_push(vm, val);
end:
  if (hk_vm_push_string(vm, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t ord_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  hk_string_t *str = hk_as_string(val);
  if (!str->length)
  {
    hk_runtime_error("type error: argument #1 must be a non-empty string");
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_number(vm, (uint32_t) str->chars[0]);
}

static int32_t chr_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_value_t val = args[1];
  if (hk_vm_check_int(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  int32_t data = (int32_t) hk_as_number(val);
  if (data < 0 || data > UCHAR_MAX)
  {
    hk_runtime_error("range error: argument #1 must be between 0 and %d", UCHAR_MAX);
    return HK_STATUS_ERROR;
  }
  hk_string_t *str = hk_string_new_with_capacity(1);
  str->length = 1;
  str->chars[0] = (char) data;
  str->chars[1] = '\0';
  if (hk_vm_push_string(vm, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t hex_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  if (!str->length)
    return hk_vm_push_string(vm, str);
  int32_t length = str->length << 1;
  hk_string_t *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  char *chars = result->chars;
  for (int32_t i = 0; i < str->length; ++i)
  {
    snprintf(chars, INT32_MAX, "%.2x", (unsigned char) str->chars[i]);
    chars += 2;
  }
  if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t bin_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  if (!str->length)
    return hk_vm_push_string(vm, str);
  if (str->length % 2)
  {
    hk_vm_push_nil(vm);
    return HK_STATUS_OK;
  }
  int32_t length = str->length >> 1;
  hk_string_t *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  char *chars = str->chars;
  for (int32_t i = 0; i < length; ++i)
  {
    sscanf(chars, "%2hhx", (unsigned char *) &result->chars[i]);
    chars += 2;
  }
  if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t address_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_value_t val = args[1];
  void *ptr = (int64_t) hk_is_object(val) ? val.as.pointer_value : NULL;
  hk_string_t *result = hk_string_new_with_capacity(32);
  char *chars = result->chars;
  snprintf(chars, 31,  "%p", ptr);
  result->length = (int32_t) strnlen(chars, 31);
  if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t refcount_call(hk_vm_t *vm, hk_value_t *args)
{
  int32_t result = hk_value_ref_count(args[1]);
  return hk_vm_push_number(vm, result);
}

static int32_t cap_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_type_t types[] = {HK_TYPE_STRING, HK_TYPE_ARRAY};
  if (hk_vm_check_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  int32_t capacity = hk_is_string(val) ? hk_as_string(val)->capacity
    : hk_as_array(val)->capacity;
  return hk_vm_push_number(vm, capacity);
}

static int32_t len_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_type_t types[] = {HK_TYPE_STRING, HK_TYPE_RANGE, HK_TYPE_ARRAY,
    HK_TYPE_STRUCT, HK_TYPE_INSTANCE};
  if (hk_vm_check_types(args, 1, 5, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  if (hk_is_string(val))
    return hk_vm_push_number(vm, hk_as_string(val)->length);
  if (hk_is_range(val))
  {
    hk_range_t *range = hk_as_range(val);
    if (range->start < range->end)
    {
      int32_t result = (int32_t) range->end - range->start + 1;
      return hk_vm_push_number(vm, result);
    }
    if (range->start > range->end)
    {
      int32_t result = (int32_t) range->start - range->end + 1;
      return hk_vm_push_number(vm, result);
    }
    return hk_vm_push_number(vm, 1);
  }
  if (hk_is_array(val))
    return hk_vm_push_number(vm, hk_as_array(val)->length);
  if (hk_is_struct(val))
    return hk_vm_push_number(vm, hk_as_struct(val)->length);
  return hk_vm_push_number(vm, hk_as_instance(val)->ztruct->length);
}

static int32_t is_empty_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_type_t types[] = {HK_TYPE_STRING, HK_TYPE_RANGE, HK_TYPE_ARRAY,
    HK_TYPE_STRUCT, HK_TYPE_INSTANCE};
  if (hk_vm_check_types(args, 1, 5, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  if (hk_is_string(val))
    return hk_vm_push_bool(vm, !hk_as_string(val)->length);
  if (hk_is_range(val))
    return hk_vm_push_bool(vm, false);
  if (hk_is_array(val))
    return hk_vm_push_bool(vm, !hk_as_array(val)->length);
  if (hk_is_struct(val))
    return hk_vm_push_bool(vm, !hk_as_struct(val)->length);
  return hk_vm_push_bool(vm, !hk_as_instance(val)->ztruct->length);
}

static int32_t compare_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_value_t val1 = args[1];
  hk_value_t val2 = args[2];
  int32_t result;
  if (hk_vm_compare(vm, val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, result);
}

static int32_t split_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_type(args, 1, HK_TYPE_STRING) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_type(args, 2, HK_TYPE_STRING) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_array_t *arr = split(hk_as_string(args[1]), hk_as_string(args[2]));
  if (hk_vm_push_array(vm, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t join_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_type(args, 1, HK_TYPE_ARRAY) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_type(args, 2, HK_TYPE_STRING) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str;
  if (join(hk_as_array(args[1]), hk_as_string(args[2]), &str) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string(vm, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t iter_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_type_t types[] = {HK_TYPE_ITERATOR, HK_TYPE_RANGE, HK_TYPE_ARRAY};
  if (hk_vm_check_types(args, 1, 3, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  if (hk_is_iterator(val))
  {
    if (hk_vm_push_iterator(vm, hk_as_iterator(val)) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    return HK_STATUS_OK;
  }
  hk_iterator_t *it = hk_new_iterator(val);
  hk_assert(it, "could not create iterator");
  if (hk_vm_push_iterator(vm, it) == HK_STATUS_ERROR)
  {
    hk_iterator_free(it);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t valid_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_type(args, 1, HK_TYPE_ITERATOR) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_bool(vm, hk_iterator_is_valid(hk_as_iterator(args[1])));
}

static int32_t current_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_type(args, 1, HK_TYPE_ITERATOR) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_iterator_t *it = hk_as_iterator(args[1]);
  if (!hk_iterator_is_valid(it))
    return hk_vm_push_nil(vm);
  return hk_vm_push(vm, hk_iterator_get_current(it));
}

static int32_t next_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_type(args, 1, HK_TYPE_ITERATOR) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_iterator_t *it = hk_as_iterator(args[1]);
  if (hk_iterator_is_valid(it))
    it = hk_iterator_next(it);
  return hk_vm_push_iterator(vm, it);
}

static int32_t sleep_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_int(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  int32_t ms = (int32_t) hk_as_number(args[1]);
#ifdef _WIN32
  Sleep(ms);
#else
  hk_assert(!usleep(ms * 1000), "unexpected error on usleep()");
#endif
  return hk_vm_push_nil(vm);
}

static int32_t assert_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_is_falsey(args[1]))
  {
    hk_string_t *str = hk_as_string(args[2]);
    fprintf(stderr, "assertion failed: %.*s\n", str->length, str->chars);
    return HK_STATUS_NO_TRACE;
  }
  return hk_vm_push_nil(vm);
}

static int32_t panic_call(hk_vm_t *vm, hk_value_t *args)
{
  (void) vm;
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  fprintf(stderr, "panic: %.*s\n", str->length, str->chars);
  return HK_STATUS_NO_TRACE;
}

void load_globals(hk_vm_t *vm)
{
  hk_vm_push_new_native(vm, globals[0], 1, &print_call);
  hk_vm_push_new_native(vm, globals[1], 1, &println_call);
  hk_vm_push_new_native(vm, globals[2], 1, &type_call);
  hk_vm_push_new_native(vm, globals[3], 1, &is_nil_call);
  hk_vm_push_new_native(vm, globals[4], 1, &is_bool_call);
  hk_vm_push_new_native(vm, globals[5], 1, &is_number_call);
  hk_vm_push_new_native(vm, globals[6], 1, &is_int_call);
  hk_vm_push_new_native(vm, globals[7], 1, &is_string_call);
  hk_vm_push_new_native(vm, globals[8], 1, &is_range_call);
  hk_vm_push_new_native(vm, globals[9], 1, &is_array_call);
  hk_vm_push_new_native(vm, globals[10], 1, &is_struct_call);
  hk_vm_push_new_native(vm, globals[11], 1, &is_instance_call);
  hk_vm_push_new_native(vm, globals[12], 1, &is_iterator_call);
  hk_vm_push_new_native(vm, globals[13], 1, &is_callable_call);
  hk_vm_push_new_native(vm, globals[14], 1, &is_userdata_call);
  hk_vm_push_new_native(vm, globals[15], 1, &is_object_call);
  hk_vm_push_new_native(vm, globals[16], 1, &is_comparable_call);
  hk_vm_push_new_native(vm, globals[17], 1, &is_iterable_call);
  hk_vm_push_new_native(vm, globals[18], 1, &to_bool_call);
  hk_vm_push_new_native(vm, globals[19], 1, &to_int_call);
  hk_vm_push_new_native(vm, globals[20], 1, &to_number_call);
  hk_vm_push_new_native(vm, globals[21], 1, &to_string_call);
  hk_vm_push_new_native(vm, globals[22], 1, &ord_call);
  hk_vm_push_new_native(vm, globals[23], 1, &chr_call);
  hk_vm_push_new_native(vm, globals[24], 1, &hex_call);
  hk_vm_push_new_native(vm, globals[25], 1, &bin_call);
  hk_vm_push_new_native(vm, globals[26], 1, &address_call);
  hk_vm_push_new_native(vm, globals[27], 1, &refcount_call);
  hk_vm_push_new_native(vm, globals[28], 1, &cap_call);
  hk_vm_push_new_native(vm, globals[29], 1, &len_call);
  hk_vm_push_new_native(vm, globals[30], 1, &is_empty_call);
  hk_vm_push_new_native(vm, globals[31], 2, &compare_call);
  hk_vm_push_new_native(vm, globals[32], 2, &split_call);
  hk_vm_push_new_native(vm, globals[33], 2, &join_call);
  hk_vm_push_new_native(vm, globals[34], 1, &iter_call);
  hk_vm_push_new_native(vm, globals[35], 1, &valid_call);
  hk_vm_push_new_native(vm, globals[36], 1, &current_call);
  hk_vm_push_new_native(vm, globals[37], 1, &next_call);
  hk_vm_push_new_native(vm, globals[38], 1, &sleep_call);
  hk_vm_push_new_native(vm, globals[39], 2, &assert_call);
  hk_vm_push_new_native(vm, globals[40], 1, &panic_call);
}

int32_t num_globals(void)
{
  return (int32_t) (sizeof(globals) / sizeof(*globals));
}

int32_t lookup_global(int32_t length, char *chars)
{
  int32_t index = num_globals() - 1;
  for (; index > -1; --index)
  {
    const char *global = globals[index];
    if (!strncmp(global, chars, length) && !global[length])
      break;
  }
  return index;
}
