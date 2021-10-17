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
  "bool",
  "int",
  "float",
  "str",
  "cap",
  "len",
  "is_empty",
  "lower",
  "upper",
  "array",
  "index_of",
  "abs",
  "floor",
  "ceil",
  "pow",
  "sqrt"
};

static inline int string_to_double(string_t *str, double *result);
static int print_call(vm_t *vm, value_t *frame);
static int println_call(vm_t *vm, value_t *frame);
static int bool_call(vm_t *vm, value_t *frame);
static int int_call(vm_t *vm, value_t *frame);
static int float_call(vm_t *vm, value_t *frame);
static int str_call(vm_t *vm, value_t *frame);
static int cap_call(vm_t *vm, value_t *frame);
static int len_call(vm_t *vm, value_t *frame);
static int is_empty_call(vm_t *vm, value_t *frame);
static int lower_call(vm_t *vm, value_t *frame);
static int upper_call(vm_t *vm, value_t *frame);
static int array_call(vm_t *vm, value_t *frame);
static int index_of_call(vm_t *vm, value_t *frame);
static int abs_call(vm_t *vm, value_t *frame);
static int floor_call(vm_t *vm, value_t *frame);
static int ceil_call(vm_t *vm, value_t *frame);
static int pow_call(vm_t *vm, value_t *frame);
static int sqrt_call(vm_t *vm, value_t *frame);

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
  vm_push_null(vm);
  return STATUS_OK;
}

static int println_call(vm_t *vm, value_t *frame)
{
  value_print(frame[1], false);
  printf("\n");
  vm_push_null(vm);
  return STATUS_OK;
}

static int bool_call(vm_t *vm, value_t *frame)
{
  vm_push_boolean(vm, IS_TRUTHY(frame[1]));
  return STATUS_OK;
}

static int int_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_NUMBER:
    vm_push_number(vm, (long) val.as.number);
    return STATUS_OK;
  case TYPE_STRING:
    {
      double result;
      if (string_to_double(AS_STRING(frame[1]), &result) == STATUS_ERROR)
        return STATUS_ERROR;
      vm_push_number(vm, (long) result);
    }
    return STATUS_OK;
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
      vm_push_number(vm, result);
    }
    return STATUS_OK;
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
  switch (val.type)
  {
  case TYPE_NULL:
    vm_push_string(vm, string_from_chars(-1, "null"));
    return STATUS_OK;
  case TYPE_BOOLEAN:
    vm_push_string(vm, string_from_chars(-1, val.as.boolean ? "true" : "false"));
    return STATUS_OK;
  case TYPE_NUMBER:
    {
      char chars[32];
      sprintf(chars, "%g", val.as.number);
      vm_push_string(vm, string_from_chars(-1, chars));
    }
    return STATUS_OK;
  case TYPE_STRING:
    return STATUS_OK;
  case TYPE_ARRAY:
  case TYPE_CALLABLE:
    break;
  }
  runtime_error("invalid type: cannot convert '%s' to 'string'", type_name(val.type));
  return STATUS_ERROR;
}

static int cap_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_STRING:
    vm_push_number(vm, AS_STRING(val)->capacity);
    return STATUS_OK;
  case TYPE_ARRAY:
    vm_push_number(vm, AS_ARRAY(val)->capacity);
    return STATUS_OK;
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_CALLABLE:
    break;
  }
  runtime_error("invalid type: '%s' has no capacity", type_name(val.type));
  return STATUS_ERROR;
}

static int len_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_STRING:
    vm_push_number(vm, AS_STRING(val)->length);
    return STATUS_OK;
  case TYPE_ARRAY:
    vm_push_number(vm, AS_ARRAY(val)->length);
    return STATUS_OK;
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_CALLABLE:
    break;
  }
  runtime_error("invalid type: '%s' has no length", type_name(val.type));
  return STATUS_ERROR;
}

static int is_empty_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_STRING:
    vm_push_boolean(vm, !AS_STRING(val)->length);
    return STATUS_OK;
  case TYPE_ARRAY:
    vm_push_boolean(vm, !AS_ARRAY(val)->length);
    return STATUS_OK;
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_CALLABLE:
    break;
  }
  runtime_error("invalid type: '%s' has no length", type_name(val.type));
  return STATUS_ERROR;
}

static int lower_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  vm_push_string(vm, string_lower(AS_STRING(val)));
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
  vm_push_string(vm, string_upper(AS_STRING(val)));
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
  vm_push_array(vm, arr);
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
  vm_push_number(vm, array_index_of(AS_ARRAY(val1), val2));
  return STATUS_OK;
}

static int abs_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  vm_push_number(vm, fabs(val.as.number));
  return STATUS_OK;
}

static int floor_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  vm_push_number(vm, floor(val.as.number));
  return STATUS_OK;
}

static int ceil_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  vm_push_number(vm, ceil(val.as.number));
  return STATUS_OK;
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
  vm_push_number(vm, pow(val1.as.number, val2.as.number));
  return STATUS_OK;
}

static int sqrt_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  vm_push_number(vm, sqrt(val.as.number));
  return STATUS_OK;
}

void globals_init(vm_t *vm)
{
  vm_push_native(vm, native_new(string_from_chars(-1, globals[0]), 1, &print_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[1]), 1, &println_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[2]), 1, &bool_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[3]), 1, &int_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[4]), 1, &float_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[5]), 1, &str_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[6]), 1, &cap_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[7]), 1, &len_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[8]), 1, &is_empty_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[9]), 1, &lower_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[10]), 1, &upper_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[11]), 1, &array_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[12]), 2, &index_of_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[13]), 1, &abs_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[14]), 1, &floor_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[15]), 1, &ceil_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[16]), 2, &pow_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[17]), 1, &sqrt_call));
}

int resolve_global(int length, char *chars)
{
  int index = sizeof(globals) / sizeof(*globals) - 1;
  for (; index > -1; --index)
  {
    const char *global = globals[index];
    if (!strncmp(global, chars, length) && !global[length])
      break;
  }
  return index;
}
