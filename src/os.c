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
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, system(AS_STRING(val)->chars));
}

static int getenv_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
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
  char clocks_per_second[] = "ClocksPerSecond";
  char clock[] = "clock";
  char system[] = "system";
  char getenv[] = "getenv";
  struct_t *ztruct = struct_new(string_from_chars(-1, "os"));
  struct_put(ztruct, sizeof(clocks_per_second) - 1, clocks_per_second);
  struct_put(ztruct, sizeof(clock) - 1, clock);
  struct_put(ztruct, sizeof(system) - 1, system);
  struct_put(ztruct, sizeof(getenv) - 1, getenv);
  vm_push_number(vm, CLOCKS_PER_SEC);
  vm_push_native(vm, native_new(string_from_chars(-1, clock), 0, &clock_call));
  vm_push_native(vm, native_new(string_from_chars(-1, system), 1, &system_call));
  vm_push_native(vm, native_new(string_from_chars(-1, getenv), 1, &getenv_call));
  vm_push_struct(vm, ztruct);
  vm_instance(vm);
}
