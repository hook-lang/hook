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
    runtime_error("invalid type: expected number but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, fabs(val.as.number));
}

static int floor_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, floor(val.as.number));
}

static int ceil_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got `%s`", type_name(val.type));
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
    runtime_error("invalid type: expected number but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_NUMBER(val2))
  {
    runtime_error("invalid type: expected number but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, pow(val1.as.number, val2.as.number));
}

static int sqrt_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_NUMBER(val))
  {
    runtime_error("invalid type: expected number but got `%s`", type_name(val.type));
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
void __declspec(dllexport) __stdcall load_math(vm_t *vm)
#else
void load_math(vm_t *vm)
#endif
{
  vm_push_string_from_chars(vm, -1, "math");
  vm_push_string_from_chars(vm, -1, "Pi");
  vm_push_number(vm, M_PI);
  vm_push_string_from_chars(vm, -1, "abs");
  vm_push_new_native(vm, "abs", 1, &abs_call);
  vm_push_string_from_chars(vm, -1, "floor");
  vm_push_new_native(vm, "floor", 1, &floor_call);
  vm_push_string_from_chars(vm, -1, "ceil");
  vm_push_new_native(vm, "ceil", 1, &ceil_call);
  vm_push_string_from_chars(vm, -1, "pow");
  vm_push_new_native(vm, "pow", 2, &pow_call);
  vm_push_string_from_chars(vm, -1, "sqrt");
  vm_push_new_native(vm, "sqrt", 1, &sqrt_call);
  vm_push_string_from_chars(vm, -1, "random");
  vm_push_new_native(vm, "random", 0, &random_call);
  ASSERT(vm_construct(vm, 7) == STATUS_OK, "cannot load library `math`");
}
