//
// Hook Programming Language
// builtin.c
//

#include "builtin.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include "common.h"
#include "error.h"

static const char *globals[] = {
  "print",
  "println",
  "type",
  "bool",
  "int",
  "float",
  "str",
  "cap",
  "len",
  "is_empty",
  "hash",
  "compare",
  "lower",
  "upper",
  "array",
  "index_of",
  "abs",
  "floor",
  "ceil",
  "pow",
  "sqrt",
  "system",
  "assert",
  "panic"
};

static inline int string_to_double(string_t *str, double *result);
static int print_call(vm_t *vm, value_t *frame);
static int println_call(vm_t *vm, value_t *frame);
static int type_call(vm_t *vm, value_t *frame);
static int bool_call(vm_t *vm, value_t *frame);
static int int_call(vm_t *vm, value_t *frame);
static int float_call(vm_t *vm, value_t *frame);
static int str_call(vm_t *vm, value_t *frame);
static int cap_call(vm_t *vm, value_t *frame);
static int len_call(vm_t *vm, value_t *frame);
static int is_empty_call(vm_t *vm, value_t *frame);
static int hash_call(vm_t *vm, value_t *frame);
static int compare_call(vm_t *vm, value_t *frame);
static int lower_call(vm_t *vm, value_t *frame);
static int upper_call(vm_t *vm, value_t *frame);
static int array_call(vm_t *vm, value_t *frame);
static int index_of_call(vm_t *vm, value_t *frame);
static int abs_call(vm_t *vm, value_t *frame);
static int floor_call(vm_t *vm, value_t *frame);
static int ceil_call(vm_t *vm, value_t *frame);
static int pow_call(vm_t *vm, value_t *frame);
static int sqrt_call(vm_t *vm, value_t *frame);
static int system_call(vm_t *vm, value_t *frame);
static int assert_call(vm_t *vm, value_t *frame);
static int panic_call(vm_t *vm, value_t *frame);

static inline int string_to_double(string_t *str, double *result)
{
  if (!str->length)
  {
    runtime_error("invalid type: cannot convert empty string to 'number'");
    return STATUS_ERROR;
  }
  errno = 0;
  char *end;
  *result = strtod(str->chars, &end);
  if (errno == ERANGE)
  {
    runtime_error("invalid type: number literal is too large");
    return STATUS_ERROR;
  }
  while (*end != 0 && isspace(*end))
    ++end;
  if (end < &str->chars[str->length])
  {
    runtime_error("invalid type: cannot convert 'string' to 'number'");
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int print_call(vm_t *vm, value_t *frame)
{
  value_print(frame[1], false);
  return vm_push_null(vm);
}

static int println_call(vm_t *vm, value_t *frame)
{
  value_print(frame[1], false);
  printf("\n");
  return vm_push_null(vm);
}

static int type_call(vm_t *vm, value_t *frame)
{
  string_t *str = string_from_chars(-1, type_name(frame[1].type));
  if (vm_push_string(vm, str) == STATUS_ERROR)
  {
    string_free(str);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int bool_call(vm_t *vm, value_t *frame)
{
  return vm_push_boolean(vm, IS_TRUTHY(frame[1]));
}

static int int_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_NUMBER:
    return vm_push_number(vm, (long) val.as.number);
  case TYPE_STRING:
    {
      double result;
      if (string_to_double(AS_STRING(frame[1]), &result) == STATUS_ERROR)
        return STATUS_ERROR;
      return vm_push_number(vm, (long) result);
    }
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_ARRAY:
  case TYPE_CALLABLE:
    break;
  }
  runtime_error("invalid type: cannot convert '%s' to 'integer'", type_name(val.type));
  return STATUS_ERROR;
}

static int float_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_NUMBER:
    return STATUS_OK;
  case TYPE_STRING:
    {
      double result;
      if (string_to_double(AS_STRING(frame[1]), &result) == STATUS_ERROR)
        return STATUS_ERROR;
      return vm_push_number(vm, result);
    }
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_ARRAY:
  case TYPE_CALLABLE:
    break;
  }
  runtime_error("invalid type: cannot convert '%s' to 'number'", type_name(val.type));
  return STATUS_ERROR;
}

static int str_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  string_t *str = NULL;
  switch (val.type)
  {
  case TYPE_NULL:
    str = string_from_chars(-1, "null");
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
    return STATUS_OK;
  case TYPE_ARRAY:
  case TYPE_CALLABLE:
    break;
  }
  if (!str)
  {
    runtime_error("invalid type: cannot convert '%s' to 'string'", type_name(val.type));
    return STATUS_ERROR;
  }
  if (vm_push_string(vm, str) == STATUS_ERROR)
  {
    string_free(str);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int cap_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_STRING:
    return vm_push_number(vm, AS_STRING(val)->capacity);
  case TYPE_ARRAY:
    return vm_push_number(vm, AS_ARRAY(val)->capacity);
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_CALLABLE:
    break;
  }
  runtime_error("invalid type: '%s' has no capacity property", type_name(val.type));
  return STATUS_ERROR;
}

static int len_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_STRING:
    return vm_push_number(vm, AS_STRING(val)->length);
  case TYPE_ARRAY:
    return vm_push_number(vm, AS_ARRAY(val)->length);
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_CALLABLE:
    break;
  }
  runtime_error("invalid type: '%s' has no length property", type_name(val.type));
  return STATUS_ERROR;
}

static int is_empty_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_STRING:
    return vm_push_boolean(vm, !AS_STRING(val)->length);
  case TYPE_ARRAY:
    return vm_push_boolean(vm, !AS_ARRAY(val)->length);
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_CALLABLE:
    break;
  }
  runtime_error("invalid type: '%s' has no length property", type_name(val.type));
  return STATUS_ERROR;
}

static int hash_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, string_hash(AS_STRING(val)));
}

static int compare_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  int result;
  if (value_compare(val1, val2, &result) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, result);
}

static int lower_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  string_t *str = string_lower(AS_STRING(val));
  if (vm_push_string(vm, str) == STATUS_ERROR)
  {
    string_free(str);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int upper_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  string_t *str = string_upper(AS_STRING(val));
  if (vm_push_string(vm, str) == STATUS_ERROR)
  {
    string_free(str);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int array_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_INTEGER(val))
  {
    runtime_error("invalid type: expected integer but got '%s'", type_name(val.type));
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
    runtime_error("invalid type: expected array but got '%s'", type_name(val1.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, array_index_of(AS_ARRAY(val1), val2));
}

static int abs_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, fabs(val.as.number));
}

static int floor_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, floor(val.as.number));
}

static int ceil_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, ceil(val.as.number));
}

static int pow_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_NUMBER(val1))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_NUMBER(val2))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val2.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, pow(val1.as.number, val2.as.number));
}

static int sqrt_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, sqrt(val.as.number));
}

static int system_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, system(AS_STRING(val)->chars));
}

static int assert_call(vm_t *vm, value_t *frame)
{
  (void) vm;
  value_t val = frame[2];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  if (IS_FALSEY(frame[1]))
  {
    string_t *str = AS_STRING(val);
    fprintf(stderr, "assertion failed: %.*s\n", str->length, str->chars);
    return STATUS_NO_TRACE;
  }
  return vm_push_null(vm);
}

static int panic_call(vm_t *vm, value_t *frame)
{
  (void) vm;
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  string_t *str = AS_STRING(val);
  fprintf(stderr, "panic: %.*s\n", str->length, str->chars);
  return STATUS_NO_TRACE;
}

void globals_init(vm_t *vm)
{
  vm_push_native(vm, native_new(string_from_chars(-1, globals[0]), 1, &print_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[1]), 1, &println_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[2]), 1, &type_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[3]), 1, &bool_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[4]), 1, &int_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[5]), 1, &float_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[6]), 1, &str_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[7]), 1, &cap_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[8]), 1, &len_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[9]), 1, &is_empty_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[10]), 1, &hash_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[11]), 2, &compare_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[12]), 1, &lower_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[13]), 1, &upper_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[14]), 1, &array_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[15]), 2, &index_of_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[16]), 1, &abs_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[17]), 1, &floor_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[18]), 1, &ceil_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[19]), 2, &pow_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[20]), 1, &sqrt_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[21]), 1, &system_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[22]), 2, &assert_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[23]), 1, &panic_call));
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
