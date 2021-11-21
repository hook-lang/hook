//
// Hook Programming Language
// strings.c
//

#include "strings.h"
#include "common.h"
#include "error.h"

static int hash_call(vm_t *vm, value_t *frame);
static int lower_call(vm_t *vm, value_t *frame);
static int upper_call(vm_t *vm, value_t *frame);
static int trim_call(vm_t *vm, value_t *frame);
static int starts_with_call(vm_t *vm, value_t *frame);
static int ends_with_call(vm_t *vm, value_t *frame);

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

static int trim_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  string_t *str;
  if (!string_trim(AS_STRING(val), &str))
    return STATUS_OK;
  if (vm_push_string(vm, str) == STATUS_ERROR)
  {
    string_free(str);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int starts_with_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_STRING(val1))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val2.type));
    return STATUS_ERROR;
  }
  return vm_push_boolean(vm, string_starts_with(AS_STRING(val1), AS_STRING(val2)));
}

static int ends_with_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_STRING(val1))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val2.type));
    return STATUS_ERROR;
  }
  return vm_push_boolean(vm, string_ends_with(AS_STRING(val1), AS_STRING(val2)));
}

#ifdef _WIN32
void __declspec(dllexport) __stdcall load_strings(vm_t *vm)
#else
void load_strings(vm_t *vm)
#endif
{
  char hash[] = "hash";
  char lower[] = "lower";
  char upper[] = "upper";
  char trim[] = "trim";
  char starts_with[] = "starts_with";
  char ends_with[] = "ends_with";
  struct_t *ztruct = struct_new(string_from_chars(-1, "strings"));
  struct_put(ztruct, sizeof(hash) - 1, hash);
  struct_put(ztruct, sizeof(lower) - 1, lower);
  struct_put(ztruct, sizeof(upper) - 1, upper);
  struct_put(ztruct, sizeof(trim) - 1, trim);
  struct_put(ztruct, sizeof(starts_with) - 1, starts_with);
  struct_put(ztruct, sizeof(ends_with) - 1, ends_with);
  vm_push_native(vm, native_new(string_from_chars(-1, hash), 1, &hash_call));
  vm_push_native(vm, native_new(string_from_chars(-1, lower), 1, &lower_call));
  vm_push_native(vm, native_new(string_from_chars(-1, upper), 1, &upper_call));
  vm_push_native(vm, native_new(string_from_chars(-1, trim), 1, &trim_call));
  vm_push_native(vm, native_new(string_from_chars(-1, starts_with), 2, &starts_with_call));
  vm_push_native(vm, native_new(string_from_chars(-1, ends_with), 2, &ends_with_call));
  vm_push_struct(vm, ztruct);
  vm_instance(vm);
}
