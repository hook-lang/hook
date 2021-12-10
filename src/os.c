//
// Hook Programming Language
// os.c
//

#include "os.h"
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "error.h"

static int clock_call(vm_t *vm, value_t *frame);
static int system_call(vm_t *vm, value_t *frame);
static int getenv_call(vm_t *vm, value_t *frame);

static int clock_call(vm_t *vm, value_t *frame)
{
  (void) frame;
  return vm_push_number(vm, (double) clock() / CLOCKS_PER_SEC);
}

static int system_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, system(AS_STRING(val)->chars));
}

static int getenv_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  const char *chars = getenv(AS_STRING(val)->chars);
  chars = chars ? chars : "";
  return vm_push_string(vm, string_from_chars(-1, chars));
}

#ifdef _WIN32
void __declspec(dllexport) __stdcall load_os(vm_t *vm)
#else
void load_os(vm_t *vm)
#endif
{
  vm_push_string(vm, string_from_chars(-1, "os"));
  vm_push_string(vm, string_from_chars(-1, "ClocksPerSecond"));
  vm_push_number(vm, CLOCKS_PER_SEC);
  vm_push_string(vm, string_from_chars(-1, "clock"));
  vm_push_native(vm, native_new(string_from_chars(-1, "clock"), 0, &clock_call));
  vm_push_string(vm, string_from_chars(-1, "system"));
  vm_push_native(vm, native_new(string_from_chars(-1, "system"), 1, &system_call));
  vm_push_string(vm, string_from_chars(-1, "getenv"));
  vm_push_native(vm, native_new(string_from_chars(-1, "getenv"), 1, &getenv_call));
  vm_construct(vm, 4);
}
