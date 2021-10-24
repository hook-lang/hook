//
// Hook Programming Language
// os.c
//

#include "os.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "error.h"

static int clock_call(vm_t *vm, value_t *frame);
static int system_call(vm_t *vm, value_t *frame);
static int getenv_call(vm_t *vm, value_t *frame);

static int clock_call(vm_t *vm, value_t *frame)
{
  (void) frame;
  return vm_push_number(vm, clock());
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
  return vm_push_string(vm, string_from_chars(-1, getenv(AS_STRING(val)->chars)));
}

#ifdef WIN32
void __declspec(dllexport) __stdcall load_os(vm_t *vm)
#else
void load_os(vm_t *vm)
#endif
{
  char clocks_per_second[] = "CLOCKS_PER_SECOND";
  char clock[] = "clock";
  char system[] = "system";
  char getenv[] = "getenv";
  struct_t *ztruct = struct_new(string_from_chars(-1, "os"));
  assert(struct_put_if_absent(ztruct, sizeof(clocks_per_second) - 1, clocks_per_second));
  assert(struct_put_if_absent(ztruct, sizeof(clock) - 1, clock));
  assert(struct_put_if_absent(ztruct, sizeof(system) - 1, system));
  assert(struct_put_if_absent(ztruct, sizeof(getenv) - 1, getenv));
  assert(vm_push_number(vm, CLOCKS_PER_SEC) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, clock), 0, &clock_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, system), 1, &system_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, getenv), 1, &getenv_call)) == STATUS_OK);
  assert(vm_push_struct(vm, ztruct) == STATUS_OK);
  vm_instance(vm);
}
