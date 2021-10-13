//
// Hook Programming Language
// builtin.c
//

#include "builtin.h"
#include <string.h>
#include <math.h>
#include <limits.h>
#include "error.h"

static const char *globals[] = {
  "print",
  "println",
  "cap",
  "len",
  "array",
  "index_of",
  "abs",
  "floor",
  "ceil",
  "pow",
  "sqrt"
};

static void print_call(vm_t *vm, value_t *frame);
static void println_call(vm_t *vm, value_t *frame);
static void cap_call(vm_t *vm, value_t *frame);
static void len_call(vm_t *vm, value_t *frame);
static void array_call(vm_t *vm, value_t *frame);
static void index_of_call(vm_t *vm, value_t *frame);
static void abs_call(vm_t *vm, value_t *frame);
static void floor_call(vm_t *vm, value_t *frame);
static void ceil_call(vm_t *vm, value_t *frame);
static void pow_call(vm_t *vm, value_t *frame);
static void sqrt_call(vm_t *vm, value_t *frame);

static void print_call(vm_t *vm, value_t *frame)
{
  value_print(frame[1], false);
  vm_push_null(vm);
}

static void println_call(vm_t *vm, value_t *frame)
{
  value_print(frame[1], false);
  printf("\n");
  vm_push_null(vm);
}

static void cap_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_STRING:
    vm_push_number(vm, AS_STRING(val)->capacity);
    return;
  case TYPE_ARRAY:
    vm_push_number(vm, AS_ARRAY(val)->capacity);
    return;
  default:
    break;
  }
  fatal_error("invalid type: '%s' has no capacity", type_name(val.type));
}

static void len_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_STRING:
    vm_push_number(vm, AS_STRING(val)->length);
    return;
  case TYPE_ARRAY:
    vm_push_number(vm, AS_ARRAY(val)->length);
    return;
  default:
    break;
  }
  fatal_error("invalid type: '%s' has no length", type_name(val.type));
}

static void array_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_INTEGER(val))
    fatal_error("invalid type: expected integer but got '%s'", type_name(val.type));
  long capacity = (long) val.as_number;
  if (capacity < 0 || capacity > INT_MAX)
    fatal_error("invalid range: capacity must be between 0 and %d", INT_MAX);
  array_t *arr = array_allocate((int) capacity);
  arr->length = 0;
  vm_push_array(vm, arr);
}

static void index_of_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_ARRAY(val1))
    fatal_error("invalid type: expected array but got '%s'", type_name(val1.type));
  vm_push_number(vm, array_index_of(AS_ARRAY(val1), val2));
}

static void abs_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
    fatal_error("invalid type: expected number but got '%s'", type_name(val.type));
  vm_push_number(vm, fabs(val.as_number));
}

static void floor_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
    fatal_error("invalid type: expected number but got '%s'", type_name(val.type));
  vm_push_number(vm, floor(val.as_number));
}

static void ceil_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
    fatal_error("invalid type: expected number but got '%s'", type_name(val.type));
  vm_push_number(vm, ceil(val.as_number));
}

static void pow_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_NUMBER(val1))
    fatal_error("invalid type: expected number but got '%s'", type_name(val1.type));
  if (!IS_NUMBER(val2))
    fatal_error("invalid type: expected number but got '%s'", type_name(val2.type));
  vm_push_number(vm, pow(val1.as_number, val2.as_number));
}

static void sqrt_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
    fatal_error("invalid type: expected number but got '%s'", type_name(val.type));
  vm_push_number(vm, sqrt(val.as_number));
}

void globals_init(vm_t *vm)
{
  vm_push_native(vm, native_new(string_from_chars(-1, globals[0]), 1, &print_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[1]), 1, &println_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[2]), 1, &cap_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[3]), 1, &len_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[4]), 1, &array_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[5]), 2, &index_of_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[6]), 1, &abs_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[7]), 1, &floor_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[8]), 1, &ceil_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[9]), 2, &pow_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[10]), 1, &sqrt_call));
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
