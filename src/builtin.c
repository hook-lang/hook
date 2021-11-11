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
  "int",
  "float",
  "str",
  "ord",
  "chr",
  "cap",
  "len",
  "is_empty",
  "compare",
  "min",
  "max",
  "sum",
  "slice",
  "split",
  "join",
  "array",
  "index_of",
  "sleep",
  "assert",
  "panic"
};

static inline int check_argument(value_t *frame, int index, type_t type);
static inline int check_argument_integer(value_t *frame, int index);
static inline int string_to_double(string_t *str, double *result);
static inline string_t *to_string(value_t val);
static inline array_t *split(string_t *str, string_t *separator);
static inline int join(array_t *arr, string_t *separator, string_t **result);
static int print_call(vm_t *vm, value_t *frame);
static int println_call(vm_t *vm, value_t *frame);
static int type_call(vm_t *vm, value_t *frame);
static int bool_call(vm_t *vm, value_t *frame);
static int int_call(vm_t *vm, value_t *frame);
static int float_call(vm_t *vm, value_t *frame);
static int str_call(vm_t *vm, value_t *frame);
static int ord_call(vm_t *vm, value_t *frame);
static int chr_call(vm_t *vm, value_t *frame);
static int cap_call(vm_t *vm, value_t *frame);
static int len_call(vm_t *vm, value_t *frame);
static int is_empty_call(vm_t *vm, value_t *frame);
static int compare_call(vm_t *vm, value_t *frame);
static int min_call(vm_t *vm, value_t *frame);
static int max_call(vm_t *vm, value_t *frame);
static int sum_call(vm_t *vm, value_t *frame);
static int slice_call(vm_t *vm, value_t *frame);
static int split_call(vm_t *vm, value_t *frame);
static int join_call(vm_t *vm, value_t *frame);
static int array_call(vm_t *vm, value_t *frame);
static int index_of_call(vm_t *vm, value_t *frame);
static int sleep_call(vm_t *vm, value_t *frame);
static int assert_call(vm_t *vm, value_t *frame);
static int panic_call(vm_t *vm, value_t *frame);

static inline int check_argument(value_t *frame, int index, type_t type)
{
  if (frame[index].type != type)
  {
    runtime_error("argument %d must be `%s`", index, type_name(type));
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static inline int check_argument_integer(value_t *frame, int index)
{
  value_t val = frame[index];
  if (!IS_INTEGER(val))
  {
    runtime_error("argument %d must be integer", index);
    return STATUS_ERROR;
  }
  long data = (long) val.as.number;
  if (data < INT_MIN || data > INT_MAX)
  {
    runtime_error("argument %d must be between %d and %d", index, INT_MIN, INT_MAX);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static inline int string_to_double(string_t *str, double *result)
{
  if (!str->length)
  {
    runtime_error("invalid type: cannot convert empty string to 'number'");
    return STATUS_ERROR;
  }
  errno = 0;
  char *ptr;
  *result = strtod(str->chars, &ptr);
  if (errno == ERANGE)
  {
    runtime_error("invalid type: number literal is too large");
    return STATUS_ERROR;
  }
  while (*ptr != 0 && isspace(*ptr))
    ++ptr;
  if (ptr < &str->chars[str->length])
  {
    runtime_error("invalid type: cannot convert 'string' to 'number'");
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static inline string_t *to_string(value_t val) {
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
    ASSERT(false, "passing string on to_string()");
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
    {
      runtime_error("array contains non-string value");
      return STATUS_ERROR;
    }
    if (i)
      string_inplace_concat(str, separator);
    string_inplace_concat(str, AS_STRING(elem));
  }
  *result = str;
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
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
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
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("invalid type: cannot convert '%s' to 'number'", type_name(val.type));
  return STATUS_ERROR;
}

static int str_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (IS_STRING(val))
    return STATUS_OK;
  string_t *str = to_string(val);
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

static int ord_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  string_t *str = AS_STRING(val);
  if (!str->length)
  {
    runtime_error("empty 'string'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, (unsigned int) str->chars[0]);
}

static int chr_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_INTEGER(val))
  {
    runtime_error("invalid type: expected integer but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  long data = (long) val.as.number;
  if (data < 0 || data > UCHAR_MAX)
  {
    runtime_error("invalid range: integer must be between 0 and %d", UCHAR_MAX);
    return STATUS_ERROR;
  }
  string_t *str = string_allocate(1);
  str->length = 1;
  str->chars[0] = (char) data;
  str->chars[1] = '\0';
  return vm_push_string(vm, str);
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
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
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
  case TYPE_STRUCT:
    return vm_push_number(vm, AS_STRUCT(val)->length);
  case TYPE_INSTANCE:
    return vm_push_number(vm, AS_INSTANCE(val)->ztruct->length);
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
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
  case TYPE_STRUCT:
    return vm_push_boolean(vm, !AS_STRUCT(val)->length);
  case TYPE_INSTANCE:
    return vm_push_boolean(vm, !AS_INSTANCE(val)->ztruct->length);
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("invalid type: '%s' has no length property", type_name(val.type));
  return STATUS_ERROR;
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

static int min_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_ARRAY(val))
  {
    runtime_error("invalid type: expected array but got '%s'", type_name(val.type));
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
    runtime_error("invalid type: expected array but got '%s'", type_name(val.type));
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
    runtime_error("invalid type: expected array but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val);
  double sum = 0;
  for (int i = 0; i < arr->length; ++i)
  {
    value_t elem = arr->elements[i];
    if (!IS_NUMBER(elem))
    {
      runtime_error("invalid type: expected array of numbers, found '%s' in array", type_name(elem.type));
      return STATUS_ERROR;
    }
    sum += elem.as.number;
  }
  vm_push_number(vm, sum);
  return STATUS_OK;
}

static int slice_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  switch (val.type)
  {
  case TYPE_STRING:
    {
      if (check_argument_integer(frame, 2) == STATUS_ERROR
       || check_argument_integer(frame, 3) == STATUS_ERROR)
        return STATUS_ERROR;
      string_t *str = AS_STRING(val);
      int start = (int) frame[2].as.number;
      int stop = (int) frame[3].as.number;
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
    }
    return STATUS_OK;
  case TYPE_ARRAY:
    {
      if (check_argument_integer(frame, 2) == STATUS_ERROR
       || check_argument_integer(frame, 3) == STATUS_ERROR)
        return STATUS_ERROR;
      array_t *arr = AS_ARRAY(val);
      int start = (int) frame[2].as.number;
      int stop = (int) frame[3].as.number;
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
    }
    return STATUS_OK;
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("invalid type: cannot slice value of type '%s'", type_name(val.type));
  return STATUS_ERROR;
}

static int split_call(vm_t *vm, value_t *frame)
{
  if (check_argument(frame, 1, TYPE_STRING) == STATUS_ERROR)
    return STATUS_ERROR;
  if (check_argument(frame, 2, TYPE_STRING) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_array(vm, split(AS_STRING(frame[1]), AS_STRING(frame[2])));
}

static int join_call(vm_t *vm, value_t *frame)
{
  if (check_argument(frame, 1, TYPE_ARRAY) == STATUS_ERROR)
    return STATUS_ERROR;
  if (check_argument(frame, 2, TYPE_STRING) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str;
  if (join(AS_ARRAY(frame[1]), AS_STRING(frame[2]), &str) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_string(vm, str);
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

static int sleep_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_INTEGER(val))
  {
    runtime_error("invalid type: expected integer but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  long ms = (long) val.as.number;
  if (ms < 0 || ms > INT_MAX)
  {
    runtime_error("invalid range: argument must be between 0 and %d", INT_MAX);
    return STATUS_ERROR;
  }
#ifdef _WIN32
  Sleep((int) ms);
#else
  ASSERT(!usleep((int) ms * 1000), "unexpected error on usleep()");
#endif
  return vm_push_null(vm);
}

static int assert_call(vm_t *vm, value_t *frame)
{
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

void load_globals(vm_t *vm)
{
  vm_push_native(vm, native_new(string_from_chars(-1, globals[0]), 1, &print_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[1]), 1, &println_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[2]), 1, &type_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[3]), 1, &bool_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[4]), 1, &int_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[5]), 1, &float_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[6]), 1, &str_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[7]), 1, &ord_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[8]), 1, &chr_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[9]), 1, &cap_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[10]), 1, &len_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[11]), 1, &is_empty_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[12]), 2, &compare_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[13]), 1, &min_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[14]), 1, &max_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[15]), 1, &sum_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[16]), 3, &slice_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[17]), 2, &split_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[18]), 2, &join_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[19]), 1, &array_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[20]), 2, &index_of_call));
  vm_push_native(vm, native_new(string_from_chars(-1, globals[21]), 1, &sleep_call));
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
