//
// Hook Programming Language
// hook_strings.c
//

#include "hook_strings.h"
#include <stdlib.h>

static int hash_call(hk_vm_t *vm, hk_value_t *args);
static int lower_call(hk_vm_t *vm, hk_value_t *args);
static int upper_call(hk_vm_t *vm, hk_value_t *args);
static int trim_call(hk_vm_t *vm, hk_value_t *args);
static int starts_with_call(hk_vm_t *vm, hk_value_t *args);
static int ends_with_call(hk_vm_t *vm, hk_value_t *args);

static int hash_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, hk_string_hash(hk_as_string(args[1])));
}

static int lower_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_string_lower(hk_as_string(args[1]));
  if (hk_vm_push_string(vm, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int upper_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_string_upper(hk_as_string(args[1]));
  if (hk_vm_push_string(vm, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int trim_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str;
  if (!hk_string_trim(hk_as_string(args[1]), &str))
    return HK_STATUS_OK;
  if (hk_vm_push_string(vm, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int starts_with_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_boolean(vm, hk_string_starts_with(hk_as_string(args[1]), hk_as_string(args[2])));
}

static int ends_with_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_boolean(vm, hk_string_ends_with(hk_as_string(args[1]), hk_as_string(args[2])));
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_strings(hk_vm_t *vm)
#else
int load_strings(hk_vm_t *vm)
#endif
{
  if (hk_vm_push_string_from_chars(vm, -1, "strings") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "hash") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "hash", 1, &hash_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "lower") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "lower", 1, &lower_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "upper") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "upper", 1, &upper_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "trim") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "trim", 1, &trim_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "starts_with") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "starts_with", 2, &starts_with_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "ends_with") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "ends_with", 2, &ends_with_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 6);
}
