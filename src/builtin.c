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
  "float",
  "str",
  "ord",
  "chr",
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
static int float_call(vm_t *vm, value_t *args);
static int str_call(vm_t *vm, value_t *args);
static int ord_call(vm_t *vm, value_t *args);
static int chr_call(vm_t *vm, value_t *args);
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
  value_t val = args[1];
  switch (val.type)
  {
  case TYPE_NUMBER:
    return vm_push_number(vm, (long) val.as.number);
  case TYPE_STRING:
    {
      double result;
      if (string_to_double(AS_STRING(args[1]), &result) == STATUS_ERROR)
        return STATUS_ERROR;
      return vm_push_number(vm, (long) result);
    }
  case TYPE_NIL:
  case TYPE_BOOLEAN:
  case TYPE_ARRAY:
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("type error: cannot convert `%s` to 'integer'", type_name(val.type));
  return STATUS_ERROR;
}

static int int_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  switch (val.type)
  {
  case TYPE_NUMBER:
    return vm_push_number(vm, (int) val.as.number);
  case TYPE_STRING:
    {
      double result;
      if (string_to_double(AS_STRING(args[1]), &result) == STATUS_ERROR)
        return STATUS_ERROR;
      return vm_push_number(vm, (int) result);
    }
  case TYPE_NIL:
  case TYPE_BOOLEAN:
  case TYPE_ARRAY:
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("type error: cannot convert `%s` to 'int'", type_name(val.type));
  return STATUS_ERROR;
}

static int float_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  switch (val.type)
  {
  case TYPE_NUMBER:
    return STATUS_OK;
  case TYPE_STRING:
    {
      double result;
      if (string_to_double(AS_STRING(args[1]), &result) == STATUS_ERROR)
        return STATUS_ERROR;
      return vm_push_number(vm, result);
    }
  case TYPE_NIL:
  case TYPE_BOOLEAN:
  case TYPE_ARRAY:
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("type error: cannot convert `%s` to 'number'", type_name(val.type));
  return STATUS_ERROR;
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
    runtime_error("type error: argument #1 must be between 0 and %d", UCHAR_MAX);
    return STATUS_ERROR;
  }
  string_t *str = string_allocate(1);
  str->length = 1;
  str->chars[0] = (char) data;
  str->chars[1] = '\0';
  return vm_push_string(vm, str);
}

static int cap_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  switch (val.type)
  {
  case TYPE_STRING:
    return vm_push_number(vm, AS_STRING(val)->capacity);
  case TYPE_ARRAY:
    return vm_push_number(vm, AS_ARRAY(val)->capacity);
  case TYPE_NIL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("type error: `%s` has no capacity property", type_name(val.type));
  return STATUS_ERROR;
}

static int len_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
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
  case TYPE_NIL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("type error: `%s` has no length property", type_name(val.type));
  return STATUS_ERROR;
}

static int is_empty_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
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
  case TYPE_NIL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("type error: `%s` has no length property", type_name(val.type));
  return STATUS_ERROR;
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
  value_t val = args[1];
  switch (val.type)
  {
  case TYPE_STRING:
    {
      if (vm_check_int(args, 2) == STATUS_ERROR
       || vm_check_int(args, 3) == STATUS_ERROR)
        return STATUS_ERROR;
      string_t *str = AS_STRING(val);
      int start = (int) args[2].as.number;
      int stop = (int) args[3].as.number;
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
      if (vm_check_int(args, 2) == STATUS_ERROR
       || vm_check_int(args, 3) == STATUS_ERROR)
        return STATUS_ERROR;
      array_t *arr = AS_ARRAY(val);
      int start = (int) args[2].as.number;
      int stop = (int) args[3].as.number;
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
  case TYPE_NIL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("type error: cannot slice value of type `%s`", type_name(val.type));
  return STATUS_ERROR;
}

static int split_call(vm_t *vm, value_t *args)
{
  if (vm_check_type(args, 1, TYPE_STRING) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_type(args, 2, TYPE_STRING) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_array(vm, split(AS_STRING(args[1]), AS_STRING(args[2])));
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
  return vm_push_string(vm, str);
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
  vm_push_new_native(vm, globals[6], 1, &float_call);
  vm_push_new_native(vm, globals[7], 1, &str_call);
  vm_push_new_native(vm, globals[8], 1, &ord_call);
  vm_push_new_native(vm, globals[9], 1, &chr_call);
  vm_push_new_native(vm, globals[10], 1, &cap_call);
  vm_push_new_native(vm, globals[11], 1, &len_call);
  vm_push_new_native(vm, globals[12], 1, &is_empty_call);
  vm_push_new_native(vm, globals[13], 2, &compare_call);
  vm_push_new_native(vm, globals[14], 3, &slice_call);
  vm_push_new_native(vm, globals[15], 2, &split_call);
  vm_push_new_native(vm, globals[16], 2, &join_call);
  vm_push_new_native(vm, globals[17], 1, &sleep_call);
  vm_push_new_native(vm, globals[18], 2, &assert_call);
  vm_push_new_native(vm, globals[19], 1, &panic_call);
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
