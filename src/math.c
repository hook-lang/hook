//
// Hook Programming Language
// math.c
//

#include "math.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "common.h"
#include "error.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int abs_call(vm_t *vm, value_t *args);
static int floor_call(vm_t *vm, value_t *args);
static int ceil_call(vm_t *vm, value_t *args);
static int pow_call(vm_t *vm, value_t *args);
static int sqrt_call(vm_t *vm, value_t *args);
static int random_call(vm_t *vm, value_t *args);

static int abs_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("type error: expected number but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, fabs(val.as.number));
}

static int floor_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("type error: expected number but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, floor(val.as.number));
}

static int ceil_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("type error: expected number but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, ceil(val.as.number));
}

static int pow_call(vm_t *vm, value_t *args)
{
  value_t val1 = args[1];
  value_t val2 = args[2];
  if (!IS_NUMBER(val1))
  {
    runtime_error("type error: expected number but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_NUMBER(val2))
  {
    runtime_error("type error: expected number but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, pow(val1.as.number, val2.as.number));
}

static int sqrt_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("type error: expected number but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, sqrt(val.as.number));
}

static int random_call(vm_t *vm, value_t *args)
{
  (void) args;
  srand((unsigned) time(NULL));
  double result = (double) rand() / RAND_MAX;
  return vm_push_number(vm, result);
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_math(vm_t *vm)
#else
int load_math(vm_t *vm)
#endif
{
  if (vm_push_string_from_chars(vm, -1, "math") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "Pi") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_number(vm, M_PI) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "abs") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "abs", 1, &abs_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "floor") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "floor", 1, &floor_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "ceil") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "ceil", 1, &ceil_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "pow") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "pow", 2, &pow_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "sqrt") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "sqrt", 1, &sqrt_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "random") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "random", 0, &random_call) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_construct(vm, 7);
}
