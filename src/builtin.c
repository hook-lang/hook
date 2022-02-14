//
// Hook Programming Language
// builtin.c
//

#include "builtin.h"
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
#include "struct.h"
#include "common.h"
#include "error.h"

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
  "sleep",
  "assert",
  "panic"
};

static inline int string_to_double(string_t *str, double *result);
static inline string_t *to_string(value_t val);
static inline array_t *split(string_t *str, string_t *separator);
static inline int join(array_t *arr, string_t *separator, string_t **result);
static int print_call(vm_t *vm, value_t *args);
static int println_call(vm_t *vm, value_t *args);
static int type_call(vm_t *vm, value_t *args);
static int bool_call(vm_t *vm, value_t *args);
static int integer_call(vm_t *vm, value_t *args);
static int int_call(vm_t *vm, value_t *args);
static int num_call(vm_t *vm, value_t *args);
static int str_call(vm_t *vm, value_t *args);
static int ord_call(vm_t *vm, value_t *args);
static int chr_call(vm_t *vm, value_t *args);
static int hex_call(vm_t *vm, value_t *args);
static int bin_call(vm_t *vm, value_t *args);
static int cap_call(vm_t *vm, value_t *args);
static int len_call(vm_t *vm, value_t *args);
static int is_empty_call(vm_t *vm, value_t *args);
static int compare_call(vm_t *vm, value_t *args);
static int slice_call(vm_t *vm, value_t *args);
static int split_call(vm_t *vm, value_t *args);
static int join_call(vm_t *vm, value_t *args);
static int sleep_call(vm_t *vm, value_t *args);
static int assert_call(vm_t *vm, value_t *args);
static int panic_call(vm_t *vm, value_t *args);

static inline int string_to_double(string_t *str, double *result)
{
  if (!str->length)
  {
    runtime_error("type error: cannot convert empty string to 'number'");
    return STATUS_ERROR;
  }
  errno = 0;
  char *ptr;
  *result = strtod(str->chars, &ptr);
  if (errno == ERANGE)
  {
    runtime_error("type error: number literal is too large");
    return STATUS_ERROR;
  }
  while (*ptr != 0 && isspace(*ptr))
    ++ptr;
  if (ptr < &str->chars[str->length])
  {
    runtime_error("type error: cannot convert 'string' to 'number'");
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static inline string_t *to_string(value_t val) {
  string_t *str = NULL;
  switch (val.type)
  {
  case TYPE_NIL:
    str = string_from_chars(-1, "nil");
    break;
  case TYPE_BOOLEAN:
    str = string_from_chars(-1, val.as.boolean ? "true" : "false");
    break;
  case TYPE_NUMBER:
    {
      char chars[32];
      sprintf(chars, "%g", val.as.number);
      str = string_from_chars(-1, chars);
    }
    break;
  case TYPE_STRING:
    ASSERT(false, "passing string on to_string()");
  case TYPE_RANGE:
  case TYPE_ARRAY:
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  return str;
}

static inline array_t *split(string_t *str, string_t *separator)
{
  array_t *arr = array_new(0);
  char *cur = str->chars;
  char *tk;
  while ((tk = strtok_r(cur, separator->chars, &cur)))
  {
    value_t elem = STRING_VALUE(string_from_chars(-1, tk));
    array_inplace_add_element(arr, elem);
  }
  return arr;
}

static inline int join(array_t *arr, string_t *separator, string_t **result)
{
  string_t *str = string_new(0);
  for (int i = 0; i < arr->length; ++i)
  {
    value_t elem = arr->elements[i];
    if (!IS_STRING(elem))
      continue;
    if (i)
      string_inplace_concat(str, separator);
    string_inplace_concat(str, AS_STRING(elem));
  }
  *result = str;
  return STATUS_OK;
}

static int print_call(vm_t *vm, value_t *args)
{
  value_print(args[1], false);
  return vm_push_nil(vm);
}

static int println_call(vm_t *vm, value_t *args)
{
  value_print(args[1], false);
  printf("\n");
  return vm_push_nil(vm);
}

static int type_call(vm_t *vm, value_t *args)
{
  return vm_push_string_from_chars(vm, -1, type_name(args[1].type));
}

static int bool_call(vm_t *vm, value_t *args)
{
  return vm_push_boolean(vm, IS_TRUTHY(args[1]));
}

static int integer_call(vm_t *vm, value_t *args)
{
  type_t types[] = {TYPE_NUMBER, TYPE_STRING};
  if (vm_check_types(args, 1, 2, types) == STATUS_ERROR)
    return STATUS_ERROR;
  value_t val = args[1];
  if (val.type == TYPE_NUMBER)
    return vm_push_number(vm, (long) val.as.number);
  double result;
  if (string_to_double(AS_STRING(val), &result) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, (long) result);
}

static int int_call(vm_t *vm, value_t *args)
{
  type_t types[] = {TYPE_NUMBER, TYPE_STRING};
  if (vm_check_types(args, 1, 2, types) == STATUS_ERROR)
    return STATUS_ERROR;
  value_t val = args[1];
  if (val.type == TYPE_NUMBER)
    return vm_push_number(vm, (int) val.as.number);
  double result;
  if (string_to_double(AS_STRING(val), &result) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, (int) result);
}

static int num_call(vm_t *vm, value_t *args)
{
  type_t types[] = {TYPE_NUMBER, TYPE_STRING};
  if (vm_check_types(args, 1, 2, types) == STATUS_ERROR)
    return STATUS_ERROR;
  value_t val = args[1];
  if (val.type == TYPE_NUMBER)
    return STATUS_OK;
  double result;
  if (string_to_double(AS_STRING(args[1]), &result) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, result);
}

static int str_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (IS_STRING(val))
    return STATUS_OK;
  string_t *str = to_string(val);
  if (!str)
  {
    runtime_error("type error: cannot convert `%s` to 'string'", type_name(val.type));
    return STATUS_ERROR;
  }
  if (vm_push_string(vm, str) == STATUS_ERROR)
  {
    string_free(str);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int ord_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  value_t val = args[1];
  string_t *str = AS_STRING(val);
  if (!str->length)
  {
    runtime_error("empty 'string'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, (unsigned int) str->chars[0]);
}

static int chr_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (vm_check_int(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  int data = (int) val.as.number;
  if (data < 0 || data > UCHAR_MAX)
  {
    runtime_error("range error: argument #1 must be between 0 and %d", UCHAR_MAX);
    return STATUS_ERROR;
  }
  string_t *str = string_allocate(1);
  str->length = 1;
  str->chars[0] = (char) data;
  str->chars[1] = '\0';
  if (vm_push_string(vm, str) == STATUS_ERROR)
  {
    string_free(str);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int hex_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  if (!str->length)
    return vm_push_string(vm, str);
  int length = str->length << 1;
  string_t *result = string_allocate(length);
  result->length = length;
  result->chars[length] = '\0';
  char *chars = result->chars;
  for (int i = 0; i < str->length; ++i)
  {
    sprintf(chars, "%.2x", (unsigned char) str->chars[i]);
    chars += 2;
  }
  if (vm_push_string(vm, result) == STATUS_ERROR)
  {
    string_free(result);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int bin_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  if (!str->length)
    return vm_push_string(vm, str);
  if (str->length % 2)
  {
    vm_push_nil(vm);
    return STATUS_OK;
  }
  int length = str->length >> 1;
  string_t *result = string_allocate(length);
  result->length = length;
  result->chars[length] = '\0';
  char *chars = str->chars;
  for (int i = 0; i < length; ++i)
  {
    sscanf(chars, "%2hhx", &result->chars[i]);
    chars += 2;
  }
  if (vm_push_string(vm, result) == STATUS_ERROR)
  {
    string_free(result);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int cap_call(vm_t *vm, value_t *args)
{
  type_t types[] = {TYPE_STRING, TYPE_ARRAY};
  if (vm_check_types(args, 1, 2, types) == STATUS_ERROR)
    return STATUS_ERROR;
  value_t val = args[1];
  int capacity = val.type == TYPE_STRING ? AS_STRING(val)->capacity
    : AS_ARRAY(val)->capacity;
  return vm_push_number(vm, capacity);
}

static int len_call(vm_t *vm, value_t *args)
{
  type_t types[] = {TYPE_STRING, TYPE_RANGE, TYPE_ARRAY,
    TYPE_STRUCT, TYPE_INSTANCE};
  if (vm_check_types(args, 1, 5, types) == STATUS_ERROR)
    return STATUS_ERROR;
  value_t val = args[1];
  int result;
  switch (val.type)
  {
  case TYPE_STRING:
    result = AS_STRING(val)->length;
    break;
  case TYPE_RANGE:
    {
      range_t *range = AS_RANGE(val);
      if (range->start < range->end)
      {
        result = (int) range->end - range->start + 1;
        break;
      }
      if (range->start > range->end)
      {
        result = (int) range->start - range->end + 1;
        break;
      }
      result = 1;
    }
    break;
  case TYPE_ARRAY:
    result = AS_ARRAY(val)->length;
    break;
  case TYPE_STRUCT:
    result = AS_STRUCT(val)->length;
    break;
  default:
    result = AS_INSTANCE(val)->ztruct->length;
    break;
  }
  return vm_push_number(vm, result);
}

static int is_empty_call(vm_t *vm, value_t *args)
{
  type_t types[] = {TYPE_STRING, TYPE_RANGE, TYPE_ARRAY,
    TYPE_STRUCT, TYPE_INSTANCE};
  if (vm_check_types(args, 1, 5, types) == STATUS_ERROR)
    return STATUS_ERROR;
  value_t val = args[1];
  bool result;
  switch (val.type)
  {
  case TYPE_STRING:
    result = !AS_STRING(val)->length;
    break;
  case TYPE_RANGE:
    result = false;
    break;
  case TYPE_ARRAY:
    result = !AS_ARRAY(val)->length;
    break;
  case TYPE_STRUCT:
    result = !AS_STRUCT(val)->length;
    break;
  default:
    result = !AS_INSTANCE(val)->ztruct->length;
    break;
  }
  return vm_push_boolean(vm, result);
}

static int compare_call(vm_t *vm, value_t *args)
{
  value_t val1 = args[1];
  value_t val2 = args[2];
  int result;
  if (value_compare(val1, val2, &result) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, result);
}

static int slice_call(vm_t *vm, value_t *args)
{
  type_t types[] = {TYPE_STRING, TYPE_ARRAY};
  if (vm_check_types(args, 1, 2, types) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_int(args, 2) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_int(args, 3) == STATUS_ERROR)
    return STATUS_ERROR;
  value_t val = args[1];
  int start = (int) args[2].as.number;
  int stop = (int) args[3].as.number;
  if (val.type == TYPE_STRING)
  {
    string_t *str = AS_STRING(val);
    string_t *result;
    if (!string_slice(str, start, stop, &result))
    {
      vm_pop(vm);
      vm_pop(vm);
      return STATUS_OK;
    }
    if (vm_push_string(vm, result) == STATUS_ERROR)
    {
      string_free(result);
      return STATUS_ERROR;
    }
    return STATUS_OK;
  }
  array_t *arr = AS_ARRAY(val);
  array_t *result;
  if (!array_slice(arr, start, stop, &result))
  {
    vm_pop(vm);
    vm_pop(vm);
    return STATUS_OK;
  }
  if (vm_push_array(vm, result) == STATUS_ERROR)
  {
    array_free(result);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int split_call(vm_t *vm, value_t *args)
{
  if (vm_check_type(args, 1, TYPE_STRING) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_type(args, 2, TYPE_STRING) == STATUS_ERROR)
    return STATUS_ERROR;
  array_t *arr = split(AS_STRING(args[1]), AS_STRING(args[2]));
  if (vm_push_array(vm, arr) == STATUS_ERROR)
  {
    array_free(arr);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int join_call(vm_t *vm, value_t *args)
{
  if (vm_check_type(args, 1, TYPE_ARRAY) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_type(args, 2, TYPE_STRING) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str;
  if (join(AS_ARRAY(args[1]), AS_STRING(args[2]), &str) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string(vm, str) == STATUS_ERROR)
  {
    string_free(str);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int sleep_call(vm_t *vm, value_t *args)
{
  if (vm_check_int(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  int ms = (int) args[1].as.number;
#ifdef _WIN32
  Sleep(ms);
#else
  ASSERT(!usleep(ms * 1000), "unexpected error on usleep()");
#endif
  return vm_push_nil(vm);
}

static int assert_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 2) == STATUS_ERROR)
    return STATUS_ERROR;
  if (IS_FALSEY(args[1]))
  {
    string_t *str = AS_STRING(args[2]);
    fprintf(stderr, "assertion failed: %.*s\n", str->length, str->chars);
    return STATUS_NO_TRACE;
  }
  return vm_push_nil(vm);
}

static int panic_call(vm_t *vm, value_t *args)
{
  (void) vm;
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  fprintf(stderr, "panic: %.*s\n", str->length, str->chars);
  return STATUS_NO_TRACE;
}

void load_globals(vm_t *vm)
{
  vm_push_new_native(vm, globals[0], 1, &print_call);
  vm_push_new_native(vm, globals[1], 1, &println_call);
  vm_push_new_native(vm, globals[2], 1, &type_call);
  vm_push_new_native(vm, globals[3], 1, &bool_call);
  vm_push_new_native(vm, globals[4], 1, &integer_call);
  vm_push_new_native(vm, globals[5], 1, &int_call);
  vm_push_new_native(vm, globals[6], 1, &num_call);
  vm_push_new_native(vm, globals[7], 1, &str_call);
  vm_push_new_native(vm, globals[8], 1, &ord_call);
  vm_push_new_native(vm, globals[9], 1, &chr_call);
  vm_push_new_native(vm, globals[10], 1, &hex_call);
  vm_push_new_native(vm, globals[11], 1, &bin_call);
  vm_push_new_native(vm, globals[12], 1, &cap_call);
  vm_push_new_native(vm, globals[13], 1, &len_call);
  vm_push_new_native(vm, globals[14], 1, &is_empty_call);
  vm_push_new_native(vm, globals[15], 2, &compare_call);
  vm_push_new_native(vm, globals[16], 3, &slice_call);
  vm_push_new_native(vm, globals[17], 2, &split_call);
  vm_push_new_native(vm, globals[18], 2, &join_call);
  vm_push_new_native(vm, globals[19], 1, &sleep_call);
  vm_push_new_native(vm, globals[20], 2, &assert_call);
  vm_push_new_native(vm, globals[21], 1, &panic_call);
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
