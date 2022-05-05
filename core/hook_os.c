//
// Hook Programming Language
// hook_os.c
//

#include "hook_os.h"
#include <stdlib.h>
#include <time.h>

static int32_t clock_call(hk_vm_t *vm, hk_value_t *args);
static int32_t time_call(hk_vm_t *vm, hk_value_t *args);
static int32_t system_call(hk_vm_t *vm, hk_value_t *args);
static int32_t getenv_call(hk_vm_t *vm, hk_value_t *args);

static int32_t clock_call(hk_vm_t *vm, hk_value_t *args)
{
  (void) args;
  return hk_vm_push_float(vm, (double) clock() / CLOCKS_PER_SEC);
}

static int32_t time_call(hk_vm_t *vm, hk_value_t *args)
{
  (void) args;
  return hk_vm_push_float(vm, (double) time(NULL));
}

static int32_t system_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, system(hk_as_string(args[1])->chars));
}

static int32_t getenv_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  const char *chars = getenv(hk_as_string(args[1])->chars);
  chars = chars ? chars : "";
  return hk_vm_push_string_from_chars(vm, -1, chars);
}

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_os(hk_vm_t *vm)
#else
int32_t load_os(hk_vm_t *vm)
#endif
{
  if (hk_vm_push_string_from_chars(vm, -1, "os") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "CLOCKS_PER_SEC") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_float(vm, CLOCKS_PER_SEC) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "clock") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "clock", 0, &clock_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "time") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "time", 0, &time_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "system") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "system", 1, &system_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "getenv") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "getenv", 1, &getenv_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 5);
}
