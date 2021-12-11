//
// Hook Programming Language
// strings.c
//

#include "strings.h"
#include <stdlib.h>
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
    runtime_error("invalid type: expected string but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, string_hash(AS_STRING(val)));
}

static int lower_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val.type));
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
    runtime_error("invalid type: expected string but got `%s`", type_name(val.type));
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
    runtime_error("invalid type: expected string but got `%s`", type_name(val.type));
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
    runtime_error("invalid type: expected string but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val2.type));
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
    runtime_error("invalid type: expected string but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val2.type));
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
  vm_push_string_from_chars(vm, -1, "strings");
  vm_push_string_from_chars(vm, -1, "hash");
  vm_push_new_native(vm, "hash", 1, &hash_call);
  vm_push_string_from_chars(vm, -1, "lower");
  vm_push_new_native(vm, "lower", 1, &lower_call);
  vm_push_string_from_chars(vm, -1, "upper");
  vm_push_new_native(vm, "upper", 1, &upper_call);
  vm_push_string_from_chars(vm, -1, "trim");
  vm_push_new_native(vm, "trim", 1, &trim_call);
  vm_push_string_from_chars(vm, -1, "starts_with");
  vm_push_new_native(vm, "starts_with", 2, &starts_with_call);
  vm_push_string_from_chars(vm, -1, "ends_with");
  vm_push_new_native(vm, "ends_with", 2, &ends_with_call);
  ASSERT(vm_construct(vm, 6) == STATUS_OK, "cannot load library `strings`");
}
