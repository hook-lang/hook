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

#ifdef WIN32
void __declspec(dllexport) __stdcall load_strings(vm_t *vm)
#else
void load_strings(vm_t *vm)
#endif
{
  char hash[] = "hash";
  char lower[] = "lower";
  char upper[] = "upper";
  char trim[] = "trim";
  struct_t *ztruct = struct_new(string_from_chars(-1, "strings"));
  struct_put(ztruct, sizeof(hash) - 1, hash);
  struct_put(ztruct, sizeof(lower) - 1, lower);
  struct_put(ztruct, sizeof(upper) - 1, upper);
  struct_put(ztruct, sizeof(trim) - 1, trim);
  vm_push_native(vm, native_new(string_from_chars(-1, hash), 1, &hash_call));
  vm_push_native(vm, native_new(string_from_chars(-1, lower), 1, &lower_call));
  vm_push_native(vm, native_new(string_from_chars(-1, upper), 1, &upper_call));
  vm_push_native(vm, native_new(string_from_chars(-1, trim), 1, &trim_call));
  vm_push_struct(vm, ztruct);
  vm_instance(vm);
}
