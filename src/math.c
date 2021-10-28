//
// Hook Programming Language
// math.c
//

#include "math.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include "common.h"
#include "error.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int abs_call(vm_t *vm, value_t *frame);
static int floor_call(vm_t *vm, value_t *frame);
static int ceil_call(vm_t *vm, value_t *frame);
static int pow_call(vm_t *vm, value_t *frame);
static int sqrt_call(vm_t *vm, value_t *frame);
static int random_call(vm_t *vm, value_t *frame);

static int abs_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, fabs(val.as.number));
}

static int floor_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, floor(val.as.number));
}

static int ceil_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, ceil(val.as.number));
}

static int pow_call(vm_t *vm, value_t *frame)
{
  value_t val1 = frame[1];
  value_t val2 = frame[2];
  if (!IS_NUMBER(val1))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_NUMBER(val2))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val2.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, pow(val1.as.number, val2.as.number));
}

static int sqrt_call(vm_t *vm, value_t *frame)
{
  value_t val = frame[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, sqrt(val.as.number));
}

static int random_call(vm_t *vm, value_t *frame)
{
  (void) frame;
  srand((unsigned) time(NULL));
  double result = (double) rand() / RAND_MAX;
  return vm_push_number(vm, result);
}

#ifdef WIN32
void __declspec(dllexport) __stdcall load_math(vm_t *vm)
#else
void load_math(vm_t *vm)
#endif
{
  char pi[] = "PI";
  char abs[] = "abs";
  char floor[] = "floor";
  char ceil[] = "ceil";
  char pow[] = "pow";
  char sqrt[] = "sqrt";
  char random[] = "random";
  struct_t *ztruct = struct_new(string_from_chars(-1, "math"));
  assert(struct_put_if_absent(ztruct, sizeof(pi) - 1, pi));
  assert(struct_put_if_absent(ztruct, sizeof(abs) - 1, abs));
  assert(struct_put_if_absent(ztruct, sizeof(floor) - 1, floor));
  assert(struct_put_if_absent(ztruct, sizeof(ceil) - 1, ceil));
  assert(struct_put_if_absent(ztruct, sizeof(pow) - 1, pow));
  assert(struct_put_if_absent(ztruct, sizeof(sqrt) - 1, sqrt));
  assert(struct_put_if_absent(ztruct, sizeof(random) - 1, random));
  assert(vm_push_number(vm, M_PI) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, abs), 1, &abs_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, floor), 1, &floor_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, ceil), 1, &ceil_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, pow), 2, &pow_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, sqrt), 1, &sqrt_call)) == STATUS_OK);
  assert(vm_push_native(vm, native_new(string_from_chars(-1, random), 0, &random_call)) == STATUS_OK);
  assert(vm_push_struct(vm, ztruct) == STATUS_OK);
  vm_instance(vm);
}
