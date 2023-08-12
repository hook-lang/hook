//
// The Hook Programming Language
// builtin.c
//

#include "builtin.h"
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <hook/iterable.h>
#include <hook/struct.h>
#include <hook/utils.h>

#ifdef _WIN32
  #include <Windows.h>
#endif

#ifndef _WIN32
  #include <unistd.h>
#endif

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
  "exit",
  "assert",
  "panic"
};

static inline void string_to_double(HkState *state, HkString *str, double *result);
static inline HkArray *split(HkString *str, HkString *sep);
static inline void join(HkArray *arr, HkString *sep, HkString **result);
static void print_call(HkState *state, HkValue *args);
static void println_call(HkState *state, HkValue *args);
static void type_call(HkState *state, HkValue *args);
static void is_nil_call(HkState *state, HkValue *args);
static void is_bool_call(HkState *state, HkValue *args);
static void is_number_call(HkState *state, HkValue *args);
static void is_int_call(HkState *state, HkValue *args);
static void is_string_call(HkState *state, HkValue *args);
static void is_range_call(HkState *state, HkValue *args);
static void is_array_call(HkState *state, HkValue *args);
static void is_struct_call(HkState *state, HkValue *args);
static void is_instance_call(HkState *state, HkValue *args);
static void is_iterator_call(HkState *state, HkValue *args);
static void is_callable_call(HkState *state, HkValue *args);
static void is_userdata_call(HkState *state, HkValue *args);
static void is_object_call(HkState *state, HkValue *args);
static void is_comparable_call(HkState *state, HkValue *args);
static void is_iterable_call(HkState *state, HkValue *args);
static void to_bool_call(HkState *state, HkValue *args);
static void to_int_call(HkState *state, HkValue *args);
static void to_number_call(HkState *state, HkValue *args);
static void to_string_call(HkState *state, HkValue *args);
static void ord_call(HkState *state, HkValue *args);
static void chr_call(HkState *state, HkValue *args);
static void hex_call(HkState *state, HkValue *args);
static void bin_call(HkState *state, HkValue *args);
static void address_call(HkState *state, HkValue *args);
static void refcount_call(HkState *state, HkValue *args);
static void cap_call(HkState *state, HkValue *args);
static void len_call(HkState *state, HkValue *args);
static void is_empty_call(HkState *state, HkValue *args);
static void compare_call(HkState *state, HkValue *args);
static void split_call(HkState *state, HkValue *args);
static void join_call(HkState *state, HkValue *args);
static void iter_call(HkState *state, HkValue *args);
static void valid_call(HkState *state, HkValue *args);
static void current_call(HkState *state, HkValue *args);
static void next_call(HkState *state, HkValue *args);
static void sleep_call(HkState *state, HkValue *args);
static void exit_call(HkState *state, HkValue *args);
static void assert_call(HkState *state, HkValue *args);
static void panic_call(HkState *state, HkValue *args);

static inline void string_to_double(HkState *state, HkString *str, double *result)
{
  if (!str->length)
  {
    hk_state_runtime_error(state, "type error: argument #1 must be a non-empty string");
    return;
  }
  if (!hk_double_from_chars(result, str->chars, true))
  {
    hk_state_runtime_error(state, "type error: argument #1 is not a convertible string");
    return;
  }
}

static inline HkArray *split(HkString *str, HkString *sep)
{
  HkArray *arr = hk_array_new();
  // TODO: Do not use strtok_r and do not copy the string
  HkString *_str = hk_string_copy(str);
  char *cur = _str->chars;
  char *tk;
  while ((tk = strtok_r(cur, sep->chars, &cur)))
  {
    HkValue elem = hk_string_value(hk_string_from_chars(-1, tk));
    hk_array_inplace_add_element(arr, elem);
  }
  hk_string_free(_str);
  return arr;
}

static inline void join(HkArray *arr, HkString *sep, HkString **result)
{
  HkString *str = hk_string_new();
  for (int i = 0; i < arr->length; ++i)
  {
    HkValue elem = hk_array_get_element(arr, i);
    if (!hk_is_string(elem))
      continue;
    if (i)
      hk_string_inplace_concat(str, sep);
    hk_string_inplace_concat(str, hk_as_string(elem));
  }
  *result = str;
}

static void print_call(HkState *state, HkValue *args)
{
  hk_value_print(args[1], false);
  hk_state_push_nil(state);
}

static void println_call(HkState *state, HkValue *args)
{
  hk_value_print(args[1], false);
  printf("\n");
  hk_state_push_nil(state);
}

static void type_call(HkState *state, HkValue *args)
{
  hk_state_push_string_from_chars(state, -1, hk_type_name(args[1].type));
}

static void is_nil_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_nil(args[1]));
}

static void is_bool_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_bool(args[1]));
}

static void is_number_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_number(args[1]));
}

static void is_int_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_int(args[1]));
}

static void is_string_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_string(args[1]));
}

static void is_range_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_range(args[1]));
}

static void is_array_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_array(args[1]));
}

static void is_struct_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_struct(args[1]));
}

static void is_instance_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_instance(args[1]));
}

static void is_iterator_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_iterator(args[1]));
}

static void is_callable_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_callable(args[1]));
}

static void is_userdata_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_userdata(args[1]));
}

static void is_object_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_object(args[1]));
}

static void is_comparable_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_comparable(args[1]));
}

static void is_iterable_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_iterable(args[1]));
}

static void to_bool_call(HkState *state, HkValue *args)
{
  hk_state_push_bool(state, hk_is_truthy(args[1]));
}

static void to_int_call(HkState *state, HkValue *args)
{
  HkType types[] = {HK_TYPE_NUMBER, HK_TYPE_STRING};
  hk_state_check_argument_types(state, args, 1, 2, types);
  hk_return_if_not_ok(state);
  HkValue val = args[1];
  if (hk_is_number(val))
  {
    hk_state_push_number(state, (int64_t) hk_as_number(val));
    return;
  }
  double result;
  string_to_double(state, hk_as_string(val), &result);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, (int64_t) result);
}

static void to_number_call(HkState *state, HkValue *args)
{
  HkType types[] = {HK_TYPE_NUMBER, HK_TYPE_STRING};
  hk_state_check_argument_types(state, args, 1, 2, types);
  hk_return_if_not_ok(state);
  HkValue val = args[1];
  if (hk_is_number(val))
  {
    hk_state_push(state, val);
    return;
  }
  double result;
  string_to_double(state, hk_as_string(args[1]), &result);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, result);
}

static void to_string_call(HkState *state, HkValue *args)
{
  HkType types[] = {HK_TYPE_NIL, HK_TYPE_BOOL, HK_TYPE_NUMBER, HK_TYPE_STRING};
  hk_state_check_argument_types(state, args, 1, 4, types);
  hk_return_if_not_ok(state);
  HkValue val = args[1];
  HkString *str;
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
  hk_state_push(state, val);
  return;
end:
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

static void ord_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkValue val = args[1];
  HkString *str = hk_as_string(val);
  if (!str->length)
  {
    hk_state_runtime_error(state, "type error: argument #1 must be a non-empty string");
    return;
  }
  hk_state_push_number(state, (uint32_t) str->chars[0]);
}

static void chr_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_int(state, args, 1);
  hk_return_if_not_ok(state);
  int data = (int) hk_as_number(args[1]);
  if (data < 0 || data > UCHAR_MAX)
  {
    hk_state_runtime_error(state, "range error: argument #1 must be between 0 and %d", UCHAR_MAX);
    return;
  }
  HkString *str = hk_string_new_with_capacity(1);
  str->length = 1;
  str->chars[0] = (char) data;
  str->chars[1] = '\0';
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

static void hex_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  if (!str->length)
  {
    hk_state_push_string(state, str);
    return;
  }
  int length = str->length << 1;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  char *chars = result->chars;
  for (int i = 0; i < str->length; ++i)
  {
    snprintf(chars, INT32_MAX, "%.2x", (unsigned char) str->chars[i]);
    chars += 2;
  }
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void bin_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  if (!str->length)
  {
    hk_state_push_string(state, str);
    return;
  }
  if (str->length % 2)
  {
    hk_state_push_nil(state);
    return;
  }
  int length = str->length >> 1;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  char *chars = str->chars;
  for (int i = 0; i < length; ++i)
  {
    sscanf(chars, "%2hhx", (unsigned char *) &result->chars[i]);
    chars += 2;
  }
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void address_call(HkState *state, HkValue *args)
{
  HkValue val = args[1];
  void *ptr = (int64_t) hk_is_object(val) ? val.as.pointer : NULL;
  HkString *result = hk_string_new_with_capacity(32);
  char *chars = result->chars;
  snprintf(chars, 31,  "%p", ptr);
  result->length = (int) strnlen(chars, 31);
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void refcount_call(HkState *state, HkValue *args)
{
  HkValue val = args[1];
  int result = hk_is_object(val) ? hk_as_object(val)->refCount : 0;
  hk_state_push_number(state, result);
}

static void cap_call(HkState *state, HkValue *args)
{
  HkType types[] = {HK_TYPE_STRING, HK_TYPE_ARRAY};
  hk_state_check_argument_types(state, args, 1, 2, types);
  hk_return_if_not_ok(state);
  HkValue val = args[1];
  int capacity = hk_is_string(val) ? hk_as_string(val)->capacity
    : hk_as_array(val)->capacity;
  hk_state_push_number(state, capacity);
}

static void len_call(HkState *state, HkValue *args)
{
  HkType types[] = {HK_TYPE_STRING, HK_TYPE_RANGE, HK_TYPE_ARRAY,
    HK_TYPE_STRUCT, HK_TYPE_INSTANCE};
  hk_state_check_argument_types(state, args, 1, 5, types);
  hk_return_if_not_ok(state);
  HkValue val = args[1];
  if (hk_is_string(val))
  {
    hk_state_push_number(state, hk_as_string(val)->length);
    return;
  }
  if (hk_is_range(val))
  {
    HkRange *range = hk_as_range(val);
    if (range->start < range->end)
    {
      int result = (int) range->end - range->start + 1;
      hk_state_push_number(state, result);
      return;
    }
    if (range->start > range->end)
    {
      int result = (int) range->start - range->end + 1;
      hk_state_push_number(state, result);
      return;
    }
    hk_state_push_number(state, 1);
    return;
  }
  if (hk_is_array(val))
  {
    hk_state_push_number(state, hk_as_array(val)->length);
    return;
  }
  if (hk_is_struct(val))
  {
    hk_state_push_number(state, hk_as_struct(val)->length);
    return;
  }
  hk_state_push_number(state, hk_as_instance(val)->ztruct->length);
}

static void is_empty_call(HkState *state, HkValue *args)
{
  HkType types[] = {HK_TYPE_STRING, HK_TYPE_RANGE, HK_TYPE_ARRAY,
    HK_TYPE_STRUCT, HK_TYPE_INSTANCE};
  hk_state_check_argument_types(state, args, 1, 5, types);
  hk_return_if_not_ok(state);
  HkValue val = args[1];
  if (hk_is_string(val))
  {
    hk_state_push_bool(state, !hk_as_string(val)->length);
    return;
  }
  if (hk_is_range(val))
  {
    hk_state_push_bool(state, false);
    return;
  }
  if (hk_is_array(val))
  {
    hk_state_push_bool(state, !hk_as_array(val)->length);
    return;
  }
  if (hk_is_struct(val))
  {
    hk_state_push_bool(state, !hk_as_struct(val)->length);
    return;
  }
  hk_state_push_bool(state, !hk_as_instance(val)->ztruct->length);
}

static void compare_call(HkState *state, HkValue *args)
{
  HkValue val1 = args[1];
  HkValue val2 = args[2];
  int result;
  hk_state_compare(state, val1, val2, &result);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, result);
}

static void split_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_type(state, args, 1, HK_TYPE_STRING);
  hk_return_if_not_ok(state);
  hk_state_check_argument_type(state, args, 2, HK_TYPE_STRING);
  hk_return_if_not_ok(state);
  HkArray *arr = split(hk_as_string(args[1]), hk_as_string(args[2]));
  hk_state_push_array(state, arr);
  if (!hk_state_is_ok(state))
    hk_array_free(arr);
}

static void join_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_type(state, args, 1, HK_TYPE_ARRAY);
  hk_return_if_not_ok(state);
  hk_state_check_argument_type(state, args, 2, HK_TYPE_STRING);
  hk_return_if_not_ok(state);
  HkString *str;
  join(hk_as_array(args[1]), hk_as_string(args[2]), &str);
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

static void iter_call(HkState *state, HkValue *args)
{
  HkType types[] = {HK_TYPE_ITERATOR, HK_TYPE_RANGE, HK_TYPE_ARRAY};
  hk_state_check_argument_types(state, args, 1, 3, types);
  hk_return_if_not_ok(state);
  HkValue val = args[1];
  if (hk_is_iterator(val))
  {
    hk_state_push_iterator(state, hk_as_iterator(val));
    return;
  }
  HkIterator *it = hk_new_iterator(val);
  hk_assert(it, "could not create iterator");
  hk_state_push_iterator(state, it);
  if (!hk_state_is_ok(state))
    hk_iterator_free(it);
}

static void valid_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_type(state, args, 1, HK_TYPE_ITERATOR);
  hk_return_if_not_ok(state);
  hk_state_push_bool(state, hk_iterator_is_valid(hk_as_iterator(args[1])));
}

static void current_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_type(state, args, 1, HK_TYPE_ITERATOR);
  hk_return_if_not_ok(state);
  HkIterator *it = hk_as_iterator(args[1]);
  if (!hk_iterator_is_valid(it))
  {
    hk_state_push_nil(state);
    return;
  }  
  hk_state_push(state, hk_iterator_get_current(it));
}

static void next_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_type(state, args, 1, HK_TYPE_ITERATOR);
  hk_return_if_not_ok(state);
  HkIterator *it = hk_as_iterator(args[1]);
  if (hk_iterator_is_valid(it))
    it = hk_iterator_next(it);
  hk_state_push_iterator(state, it);
}

static void sleep_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_int(state, args, 1);
  hk_return_if_not_ok(state);
  int ms = (int) hk_as_number(args[1]);
#ifdef _WIN32
  Sleep(ms);
#else
  hk_assert(!usleep(ms * 1000), "unexpected error on usleep()");
#endif
  hk_state_push_nil(state);
}

static void exit_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_int(state, args, 1);
  hk_return_if_not_ok(state);
  int code = (int) hk_as_number(args[1]);
  state->flags |= HK_STATE_FLAG_NO_TRACE;
  state->status = HK_STATE_STATUS_EXIT;
  hk_state_push_number(state, code);
}

static void assert_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  if (hk_is_truthy(args[1]))
  {
    hk_state_push_nil(state);
    return;
  }
  state->flags |= HK_STATE_FLAG_NO_TRACE;
  state->status = HK_STATE_STATUS_ERROR;
  HkString *str = hk_as_string(args[2]);
  fprintf(stderr, "assertion failed: %.*s\n", str->length, str->chars);
}

static void panic_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  state->flags |= HK_STATE_FLAG_NO_TRACE;
  state->status = HK_STATE_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  fprintf(stderr, "panic: %.*s\n", str->length, str->chars);
}

void load_globals(HkState *state)
{
  hk_state_push_new_native(state, globals[0], 1, print_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[1], 1, println_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[2], 1, type_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[3], 1, is_nil_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[4], 1, is_bool_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[5], 1, is_number_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[6], 1, is_int_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[7], 1, is_string_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[8], 1, is_range_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[9], 1, is_array_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[10], 1, is_struct_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[11], 1, is_instance_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[12], 1, is_iterator_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[13], 1, is_callable_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[14], 1, is_userdata_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[15], 1, is_object_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[16], 1, is_comparable_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[17], 1, is_iterable_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[18], 1, to_bool_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[19], 1, to_int_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[20], 1, to_number_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[21], 1, to_string_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[22], 1, ord_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[23], 1, chr_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[24], 1, hex_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[25], 1, bin_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[26], 1, address_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[27], 1, refcount_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[28], 1, cap_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[29], 1, len_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[30], 1, is_empty_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[31], 2, compare_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[32], 2, split_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[33], 2, join_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[34], 1, iter_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[35], 1, valid_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[36], 1, current_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[37], 1, next_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[38], 1, sleep_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[39], 1, exit_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[40], 2, assert_call);
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, globals[41], 1, panic_call);
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
