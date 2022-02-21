//
// Hook Programming Language
// h_os.c
//

#include "h_os.h"
#include <stdlib.h>
#include <time.h>
#include "h_common.h"

static int clock_call(vm_t *vm, value_t *args);
static int system_call(vm_t *vm, value_t *args);
static int getenv_call(vm_t *vm, value_t *args);

static int clock_call(vm_t *vm, value_t *args)
{
  (void) args;
  return vm_push_number(vm, (double) clock() / CLOCKS_PER_SEC);
}

static int system_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, system(AS_STRING(args[1])->chars));
}

static int getenv_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  const char *chars = getenv(AS_STRING(args[1])->chars);
  chars = chars ? chars : "";
  return vm_push_string_from_chars(vm, -1, chars);
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_os(vm_t *vm)
#else
int load_os(vm_t *vm)
#endif
{
  if (vm_push_string_from_chars(vm, -1, "os") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "ClocksPerSecond") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_number(vm, CLOCKS_PER_SEC) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "clock") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "clock", 0, &clock_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "system") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "system", 1, &system_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "getenv") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "getenv", 1, &getenv_call) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_construct(vm, 4);
}
