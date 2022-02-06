//
// Hook Programming Language
// strings.c
//

#include "strings.h"
#include <stdlib.h>
#include "common.h"
#include "error.h"

static int hash_call(vm_t *vm, value_t *args);
static int lower_call(vm_t *vm, value_t *args);
static int upper_call(vm_t *vm, value_t *args);
static int trim_call(vm_t *vm, value_t *args);
static int starts_with_call(vm_t *vm, value_t *args);
static int ends_with_call(vm_t *vm, value_t *args);

static int hash_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, string_hash(AS_STRING(args[1])));
}

static int lower_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = string_lower(AS_STRING(args[1]));
  if (vm_push_string(vm, str) == STATUS_ERROR)
  {
    string_free(str);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int upper_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = string_upper(AS_STRING(args[1]));
  if (vm_push_string(vm, str) == STATUS_ERROR)
  {
    string_free(str);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int trim_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str;
  if (!string_trim(AS_STRING(args[1]), &str))
    return STATUS_OK;
  if (vm_push_string(vm, str) == STATUS_ERROR)
  {
    string_free(str);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int starts_with_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_string(args, 2) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_boolean(vm, string_starts_with(AS_STRING(args[1]), AS_STRING(args[2])));
}

static int ends_with_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_string(args, 2) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_boolean(vm, string_ends_with(AS_STRING(args[1]), AS_STRING(args[2])));
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_strings(vm_t *vm)
#else
int load_strings(vm_t *vm)
#endif
{
  if (vm_push_string_from_chars(vm, -1, "strings") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "hash") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "hash", 1, &hash_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "lower") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "lower", 1, &lower_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "upper") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "upper", 1, &upper_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "trim") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "trim", 1, &trim_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "starts_with") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "starts_with", 2, &starts_with_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "ends_with") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "ends_with", 2, &ends_with_call) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_construct(vm, 6);
}
