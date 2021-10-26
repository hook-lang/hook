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
#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif
#include <errno.h>
#include <assert.h>
#include "struct.h"
#include "common.h"
#include "error.h"

#define HOME "HOOK_HOME"

#ifdef WIN32
typedef int (__stdcall *load_library_t)(vm_t *);
#else
typedef void (*load_library_t)(vm_t *);
#endif

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
  "assert",
  "panic",
  "require"
};

static inline int string_to_double(string_t *str, double *result);
static inline int load_library(vm_t *vm, string_t *name);
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
static int assert_call(vm_t *vm, value_t *frame);
static int panic_call(vm_t *vm, value_t *frame);
static int require_call(vm_t *vm, value_t *frame);

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

static inline int load_library(vm_t *vm, string_t *name)
{
#ifdef WIN32
  const char *file_infix = "\\lib\\";
  const char *file_ext = ".dll";
#else
  const char *file_infix = "/lib/lib";
#ifdef __APPLE__
  const char *file_ext = ".dylib";
#else
  const char *file_ext = ".so";
#endif
#endif
  const char *func_prefix = "load_";
  char *home = getenv(HOME);
  if (!home)
  {
    runtime_error("environment variable `%s` not defined", HOME);
    return STATUS_ERROR;
  }
  string_t *file = string_from_chars(-1, home);
  string_inplace_concat_chars(file, -1, file_infix);
  string_inplace_concat(file, name);
  string_inplace_concat_chars(file, -1, file_ext);
#ifdef WIN32
  HINSTANCE handle = LoadLibrary(file->chars);
#else
  void *handle = dlopen(file->chars, RTLD_NOW | RTLD_GLOBAL);
#endif
  if (!handle)
  {
    runtime_error("cannot load library `%.*s`", name->length, name->chars);
    string_free(file);
    return STATUS_ERROR;
  }
  string_free(file);
  string_t *func = string_from_chars(-1, func_prefix);
  string_inplace_concat(func, name);
#ifdef WIN32
  load_library_t load = GetProcAddress(handle, func->chars);
#else
  load_library_t load = dlsym(handle, func->chars);
#endif
  if (!load)
  {
    runtime_error("no such function %.*s()", func->length, func->chars);
    string_free(func);
    return STATUS_ERROR;
  }
  string_free(func);
  load(vm);
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
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
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

static int require_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return load_library(vm, AS_STRING(val));
}

void load_globals(vm_t *vm)
{
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[0]), 1, &print_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[1]), 1, &println_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[2]), 1, &type_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[3]), 1, &bool_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[4]), 1, &int_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[5]), 1, &float_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[6]), 1, &str_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[7]), 1, &cap_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[8]), 1, &len_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[9]), 1, &is_empty_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[10]), 1, &hash_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[11]), 2, &compare_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[12]), 1, &lower_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[13]), 1, &upper_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[14]), 1, &array_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[15]), 2, &index_of_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[16]), 2, &assert_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[17]), 1, &panic_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, globals[18]), 1, &require_call)) == STATUS_OK);
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
