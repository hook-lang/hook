//
// Hook Programming Language
// math.c
//

#include "math.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "common.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int abs_call(vm_t *vm, value_t *args);
static int sin_call(vm_t *vm, value_t *args);
static int cos_call(vm_t *vm, value_t *args);
static int tan_call(vm_t *vm, value_t *args);
static int asin_call(vm_t *vm, value_t *args);
static int acos_call(vm_t *vm, value_t *args);
static int atan_call(vm_t *vm, value_t *args);
static int floor_call(vm_t *vm, value_t *args);
static int ceil_call(vm_t *vm, value_t *args);
static int pow_call(vm_t *vm, value_t *args);
static int sqrt_call(vm_t *vm, value_t *args);
static int log_call(vm_t *vm, value_t *args);
static int exp_call(vm_t *vm, value_t *args);
static int random_call(vm_t *vm, value_t *args);

static int abs_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, fabs(args[1].as.number));
}

static int sin_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, sin(args[1].as.number));
}

static int cos_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, cos(args[1].as.number));
}

static int tan_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, tan(args[1].as.number));
}

static int asin_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, asin(args[1].as.number));
}

static int acos_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, acos(args[1].as.number));
}

static int atan_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, atan(args[1].as.number));
}

static int floor_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, floor(args[1].as.number));
}

static int ceil_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, ceil(args[1].as.number));
}

static int pow_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_number(args, 2) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, pow(args[1].as.number, args[2].as.number));
}

static int sqrt_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, sqrt(args[1].as.number));
}

static int log_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, log(args[1].as.number));
}

static int exp_call(vm_t *vm, value_t *args)
{
  if (vm_check_number(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_push_number(vm, exp(args[1].as.number));
}

static int random_call(vm_t *vm, value_t *args)
{
  (void) args;
  double result = (double) rand() / RAND_MAX;
  return vm_push_number(vm, result);
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_math(vm_t *vm)
#else
int load_math(vm_t *vm)
#endif
{
  srand((unsigned) time(NULL));
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
  if (vm_push_string_from_chars(vm, -1, "sin") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "sin", 1, &sin_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "cos") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "cos", 1, &cos_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "tan") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "tan", 1, &tan_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "asin") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "asin", 1, &asin_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "acos") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "acos", 1, &acos_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "atan") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "atan", 1, &atan_call) == STATUS_ERROR)
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
  if (vm_push_string_from_chars(vm, -1, "log") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "log", 0, &log_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "exp") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "exp", 0, &exp_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "random") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "random", 0, &random_call) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_construct(vm, 15);
}
