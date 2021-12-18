//
// Hook Programming Language
// os.c
//

#include "os.h"
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "error.h"

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
  value_t val = args[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, system(AS_STRING(val)->chars));
}

static int getenv_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  const char *chars = getenv(AS_STRING(val)->chars);
  chars = chars ? chars : "";
  return vm_push_string_from_chars(vm, -1, chars);
}

#ifdef _WIN32
void __declspec(dllexport) __stdcall load_os(vm_t *vm)
#else
void load_os(vm_t *vm)
#endif
{
  vm_push_string_from_chars(vm, -1, "os");
  vm_push_string_from_chars(vm, -1, "ClocksPerSecond");
  vm_push_number(vm, CLOCKS_PER_SEC);
  vm_push_string_from_chars(vm, -1, "clock");
  vm_push_new_native(vm, "clock", 0, &clock_call);
  vm_push_string_from_chars(vm, -1, "system");
  vm_push_new_native(vm, "system", 1, &system_call);
  vm_push_string_from_chars(vm, -1, "getenv");
  vm_push_new_native(vm, "getenv", 1, &getenv_call);
  ASSERT(vm_construct(vm, 4) == STATUS_OK, "cannot load library `os`");
}
