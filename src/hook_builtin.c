//
// Hook Programming Language
// hook_builtin.c
//

#include "hook_builtin.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <ctype.h>
#include <limits.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <errno.h>
#include "hook_struct.h"
#include "hook_utils.h"
#include "hook_status.h"
#include "hook_error.h"

#ifdef _WIN32
#define strtok_r strtok_s
#endif

static const char *globals[] = {
  "print",
  "println",
  "type",
  "bool",
  "integer",
  "int",
  "num",
  "str",
  "ord",
  "chr",
  "hex",
  "bin",
  "cap",
  "len",
  "is_empty",
  "compare",
  "slice",
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

static inline int string_to_double(hk_string_t *str, double *result);
static inline hk_array_t *split(hk_string_t *str, hk_string_t *separator);
static inline int join(hk_array_t *arr, hk_string_t *separator, hk_string_t **result);
static int print_call(hk_vm_t *vm, hk_value_t *args);
static int println_call(hk_vm_t *vm, hk_value_t *args);
static int type_call(hk_vm_t *vm, hk_value_t *args);
static int bool_call(hk_vm_t *vm, hk_value_t *args);
static int integer_call(hk_vm_t *vm, hk_value_t *args);
static int int_call(hk_vm_t *vm, hk_value_t *args);
static int num_call(hk_vm_t *vm, hk_value_t *args);
static int str_call(hk_vm_t *vm, hk_value_t *args);
static int ord_call(hk_vm_t *vm, hk_value_t *args);
static int chr_call(hk_vm_t *vm, hk_value_t *args);
static int hex_call(hk_vm_t *vm, hk_value_t *args);
static int bin_call(hk_vm_t *vm, hk_value_t *args);
static int cap_call(hk_vm_t *vm, hk_value_t *args);
static int len_call(hk_vm_t *vm, hk_value_t *args);
static int is_empty_call(hk_vm_t *vm, hk_value_t *args);
static int compare_call(hk_vm_t *vm, hk_value_t *args);
static int slice_call(hk_vm_t *vm, hk_value_t *args);
static int split_call(hk_vm_t *vm, hk_value_t *args);
static int join_call(hk_vm_t *vm, hk_value_t *args);
static int iter_call(hk_vm_t *vm, hk_value_t *args);
static int valid_call(hk_vm_t *vm, hk_value_t *args);
static int current_call(hk_vm_t *vm, hk_value_t *args);
static int next_call(hk_vm_t *vm, hk_value_t *args);
static int sleep_call(hk_vm_t *vm, hk_value_t *args);
static int assert_call(hk_vm_t *vm, hk_value_t *args);
static int panic_call(hk_vm_t *vm, hk_value_t *args);

static inline int string_to_double(hk_string_t *str, double *result)
{
  if (!str->length)
  {
    hk_runtime_error("type error: argument #1 must be a non-empty string");
    return HK_STATUS_ERROR;
  }
  errno = 0;
  char *ptr;
  *result = strtod(str->chars, &ptr);
  if (errno == ERANGE)
  {
    hk_runtime_error("type error: argument #1 is a too large string");
    return HK_STATUS_ERROR;
  }
  while (*ptr != 0 && isspace(*ptr))
    ++ptr;
  if (ptr < &str->chars[str->length])
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

static inline int join(hk_array_t *arr, hk_string_t *separator, hk_string_t **result)
{
  hk_string_t *str = hk_string_new();
  for (int i = 0; i < arr->length; ++i)
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

static int print_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_value_print(args[1], false);
  return hk_vm_push_nil(vm);
}

static int println_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_value_print(args[1], false);
  printf("\n");
  return hk_vm_push_nil(vm);
}

static int type_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_string_from_chars(vm, -1, hk_type_name(args[1].type));
}

static int bool_call(hk_vm_t *vm, hk_value_t *args)
{
  return hk_vm_push_boolean(vm, hk_is_truthy(args[1]));
}

static int integer_call(hk_vm_t *vm, hk_value_t *args)
{
  int types[] = {HK_TYPE_NUMBER, HK_TYPE_STRING};
  if (hk_vm_check_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  if (hk_is_number(val))
    return hk_vm_push_number(vm, (long) val.as.number);
  double result;
  if (string_to_double(hk_as_string(val), &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, (long) result);
}

static int int_call(hk_vm_t *vm, hk_value_t *args)
{
  int types[] = {HK_TYPE_NUMBER, HK_TYPE_STRING};
  if (hk_vm_check_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  if (hk_is_number(val))
    return hk_vm_push_number(vm, (int) val.as.number);
  double result;
  if (string_to_double(hk_as_string(val), &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, (int) result);
}

static int num_call(hk_vm_t *vm, hk_value_t *args)
{
  int types[] = {HK_TYPE_NUMBER, HK_TYPE_STRING};
  if (hk_vm_check_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  if (hk_is_number(val))
    return HK_STATUS_OK;
  double result;
  if (string_to_double(hk_as_string(args[1]), &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, result);
}

static int str_call(hk_vm_t *vm, hk_value_t *args)
{
  int types[] = {HK_TYPE_NIL, HK_TYPE_BOOLEAN, HK_TYPE_NUMBER, HK_TYPE_STRING};
  if (hk_vm_check_types(args, 1, 4, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  hk_string_t *str;
  if (hk_is_nil(val))
  {
    str = hk_string_from_chars(-1, "nil");
    goto end;
  }
  if (hk_is_boolean(val))
  {
    str = hk_string_from_chars(-1, val.as.boolean ? "true" : "false");
    goto end;
  }
  if (hk_is_number(val))
  {
    char chars[32];
    sprintf(chars, "%g", val.as.number);
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

static int ord_call(hk_vm_t *vm, hk_value_t *args)
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
  return hk_vm_push_number(vm, (unsigned int) str->chars[0]);
}

static int chr_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_value_t val = args[1];
  if (hk_vm_check_int(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  int data = (int) val.as.number;
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

static int hex_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  if (!str->length)
    return hk_vm_push_string(vm, str);
  int length = str->length << 1;
  hk_string_t *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  char *chars = result->chars;
  for (int i = 0; i < str->length; ++i)
  {
    sprintf(chars, "%.2x", (unsigned char) str->chars[i]);
    chars += 2;
  }
  if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int bin_call(hk_vm_t *vm, hk_value_t *args)
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
  int length = str->length >> 1;
  hk_string_t *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  char *chars = str->chars;
  for (int i = 0; i < length; ++i)
  {
    sscanf(chars, "%2hhx", &result->chars[i]);
    chars += 2;
  }
  if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int cap_call(hk_vm_t *vm, hk_value_t *args)
{
  int types[] = {HK_TYPE_STRING, HK_TYPE_ARRAY};
  if (hk_vm_check_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  int capacity = hk_is_string(val) ? hk_as_string(val)->capacity
    : hk_as_array(val)->capacity;
  return hk_vm_push_number(vm, capacity);
}

static int len_call(hk_vm_t *vm, hk_value_t *args)
{
  int types[] = {HK_TYPE_STRING, HK_TYPE_RANGE, HK_TYPE_ARRAY,
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
      int result = (int) range->end - range->start + 1;
      return hk_vm_push_number(vm, result);
    }
    if (range->start > range->end)
    {
      int result = (int) range->start - range->end + 1;
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

static int is_empty_call(hk_vm_t *vm, hk_value_t *args)
{
  int types[] = {HK_TYPE_STRING, HK_TYPE_RANGE, HK_TYPE_ARRAY,
    HK_TYPE_STRUCT, HK_TYPE_INSTANCE};
  if (hk_vm_check_types(args, 1, 5, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  if (hk_is_string(val))
    return hk_vm_push_boolean(vm, !hk_as_string(val)->length);
  if (hk_is_range(val))
    return hk_vm_push_boolean(vm, false);
  if (hk_is_array(val))
    return hk_vm_push_boolean(vm, !hk_as_array(val)->length);
  if (hk_is_struct(val))
    return hk_vm_push_boolean(vm, !hk_as_struct(val)->length);
  return hk_vm_push_boolean(vm, !hk_as_instance(val)->ztruct->length);
}

static int compare_call(hk_vm_t *vm, hk_value_t *args)
{
  hk_value_t val1 = args[1];
  hk_value_t val2 = args[2];
  int result;
  if (hk_value_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, result);
}

static int slice_call(hk_vm_t *vm, hk_value_t *args)
{
  int types[] = {HK_TYPE_STRING, HK_TYPE_ARRAY};
  if (hk_vm_check_types(args, 1, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  int start = (int) args[2].as.number;
  int stop = (int) args[3].as.number;
  if (hk_is_string(val))
  {
    hk_string_t *str = hk_as_string(val);
    hk_string_t *result;
    if (!hk_string_slice(str, start, stop, &result))
    {
      hk_vm_pop(vm);
      hk_vm_pop(vm);
      return HK_STATUS_OK;
    }
    if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
    {
      hk_string_free(result);
      return HK_STATUS_ERROR;
    }
    return HK_STATUS_OK;
  }
  hk_array_t *arr = hk_as_array(val);
  hk_array_t *result;
  if (!hk_array_slice(arr, start, stop, &result))
  {
    hk_vm_pop(vm);
    hk_vm_pop(vm);
    return HK_STATUS_OK;
  }
  if (hk_vm_push_array(vm, result) == HK_STATUS_ERROR)
  {
    hk_array_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int split_call(hk_vm_t *vm, hk_value_t *args)
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

static int join_call(hk_vm_t *vm, hk_value_t *args)
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

static int iter_call(hk_vm_t *vm, hk_value_t *args)
{
  int types[] = {HK_TYPE_ITERATOR, HK_TYPE_RANGE, HK_TYPE_ARRAY};
  if (hk_vm_check_types(args, 1, 3, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t val = args[1];
  if (hk_is_iterator(val))
  {
    if (hk_vm_push_iterator(vm, hk_as_iterator(val)) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    return HK_STATUS_OK;
  }
  hk_iterator_t *it = hk_is_range(val) ? hk_range_new_iterator(hk_as_range(val))
    : hk_array_new_iterator(hk_as_array(val));
  if (hk_vm_push_iterator(vm, it) == HK_STATUS_ERROR)
  {
    hk_iterator_free(it);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int valid_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_type(args, 1, HK_TYPE_ITERATOR) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_boolean(vm, hk_iterator_is_valid(hk_as_iterator(args[1])));
}

static int current_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_type(args, 1, HK_TYPE_ITERATOR) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_iterator_t *it = hk_as_iterator(args[1]);
  if (!hk_iterator_is_valid(it))
    return hk_vm_push_nil(vm);
  return hk_vm_push(vm, hk_iterator_get_current(it));
}

static int next_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_type(args, 1, HK_TYPE_ITERATOR) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_iterator_t *it = hk_as_iterator(args[1]);
  if (hk_iterator_is_valid(it))
    hk_iterator_next(it);
  return hk_vm_push_nil(vm);
}

static int sleep_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_int(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  int ms = (int) args[1].as.number;
#ifdef _WIN32
  Sleep(ms);
#else
  hk_assert(!usleep(ms * 1000), "unexpected error on usleep()");
#endif
  return hk_vm_push_nil(vm);
}

static int assert_call(hk_vm_t *vm, hk_value_t *args)
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

static int panic_call(hk_vm_t *vm, hk_value_t *args)
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
  hk_vm_push_new_native(vm, globals[3], 1, &bool_call);
  hk_vm_push_new_native(vm, globals[4], 1, &integer_call);
  hk_vm_push_new_native(vm, globals[5], 1, &int_call);
  hk_vm_push_new_native(vm, globals[6], 1, &num_call);
  hk_vm_push_new_native(vm, globals[7], 1, &str_call);
  hk_vm_push_new_native(vm, globals[8], 1, &ord_call);
  hk_vm_push_new_native(vm, globals[9], 1, &chr_call);
  hk_vm_push_new_native(vm, globals[10], 1, &hex_call);
  hk_vm_push_new_native(vm, globals[11], 1, &bin_call);
  hk_vm_push_new_native(vm, globals[12], 1, &cap_call);
  hk_vm_push_new_native(vm, globals[13], 1, &len_call);
  hk_vm_push_new_native(vm, globals[14], 1, &is_empty_call);
  hk_vm_push_new_native(vm, globals[15], 2, &compare_call);
  hk_vm_push_new_native(vm, globals[16], 3, &slice_call);
  hk_vm_push_new_native(vm, globals[17], 2, &split_call);
  hk_vm_push_new_native(vm, globals[18], 2, &join_call);
  hk_vm_push_new_native(vm, globals[19], 1, &iter_call);
  hk_vm_push_new_native(vm, globals[20], 1, &valid_call);
  hk_vm_push_new_native(vm, globals[21], 1, &current_call);
  hk_vm_push_new_native(vm, globals[22], 1, &next_call);
  hk_vm_push_new_native(vm, globals[23], 1, &sleep_call);
  hk_vm_push_new_native(vm, globals[24], 2, &assert_call);
  hk_vm_push_new_native(vm, globals[25], 1, &panic_call);
}

int num_globals(void)
{
  return (int) (sizeof(globals) / sizeof(*globals));
}

int lookup_global(int length, char *chars)
{
  int index = num_globals() - 1;
  for (; index > -1; --index)
  {
    const char *global = globals[index];
    if (!strncmp(global, chars, length) && !global[length])
      break;
  }
  return index;
}
