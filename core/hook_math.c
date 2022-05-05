//
// Hook Programming Language
// hook_math.c
//

#include "hook_math.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int32_t abs_call(hk_vm_t *vm, hk_value_t *args);
static int32_t sin_call(hk_vm_t *vm, hk_value_t *args);
static int32_t cos_call(hk_vm_t *vm, hk_value_t *args);
static int32_t tan_call(hk_vm_t *vm, hk_value_t *args);
static int32_t asin_call(hk_vm_t *vm, hk_value_t *args);
static int32_t acos_call(hk_vm_t *vm, hk_value_t *args);
static int32_t atan_call(hk_vm_t *vm, hk_value_t *args);
static int32_t floor_call(hk_vm_t *vm, hk_value_t *args);
static int32_t ceil_call(hk_vm_t *vm, hk_value_t *args);
static int32_t round_call(hk_vm_t *vm, hk_value_t *args);
static int32_t pow_call(hk_vm_t *vm, hk_value_t *args);
static int32_t sqrt_call(hk_vm_t *vm, hk_value_t *args);
static int32_t cbrt_call(hk_vm_t *vm, hk_value_t *args);
static int32_t log_call(hk_vm_t *vm, hk_value_t *args);
static int32_t log2_call(hk_vm_t *vm, hk_value_t *args);
static int32_t log10_call(hk_vm_t *vm, hk_value_t *args);
static int32_t exp_call(hk_vm_t *vm, hk_value_t *args);
static int32_t random_call(hk_vm_t *vm, hk_value_t *args);

static int32_t abs_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, fabs(args[1].as_float));
}

static int32_t sin_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, sin(args[1].as_float));
}

static int32_t cos_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, cos(args[1].as_float));
}

static int32_t tan_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, tan(args[1].as_float));
}

static int32_t asin_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, asin(args[1].as_float));
}

static int32_t acos_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, acos(args[1].as_float));
}

static int32_t atan_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, atan(args[1].as_float));
}

static int32_t floor_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, floor(args[1].as_float));
}

static int32_t ceil_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, ceil(args[1].as_float));
}

static int32_t round_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, round(args[1].as_float));
}

static int32_t pow_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_float(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, pow(args[1].as_float, args[2].as_float));
}

static int32_t sqrt_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, sqrt(args[1].as_float));
}

static int32_t cbrt_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, cbrt(args[1].as_float));
}

static int32_t log_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, log(args[1].as_float));
}

static int32_t log2_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, log2(args[1].as_float));
}

static int32_t log10_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, log10(args[1].as_float));
}

static int32_t exp_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_float(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_float(vm, exp(args[1].as_float));
}

static int32_t random_call(hk_vm_t *vm, hk_value_t *args)
{
  (void) args;
  double result = (double) rand() / RAND_MAX;
  return hk_vm_push_float(vm, result);
}

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_math(hk_vm_t *vm)
#else
int32_t load_math(hk_vm_t *vm)
#endif
{
  srand((uint32_t) time(NULL));
  if (hk_vm_push_string_from_chars(vm, -1, "math") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "PI") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_float(vm, M_PI) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "abs") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "abs", 1, &abs_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "sin") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sin", 1, &sin_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "cos") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "cos", 1, &cos_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "tan") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "tan", 1, &tan_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "asin") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "asin", 1, &asin_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "acos") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "acos", 1, &acos_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "atan") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "atan", 1, &atan_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "floor") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "floor", 1, &floor_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "ceil") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "ceil", 1, &ceil_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "round") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "round", 1, &round_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "pow") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "pow", 2, &pow_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "sqrt") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sqrt", 1, &sqrt_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "cbrt") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "cbrt", 1, &cbrt_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "log") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "log", 0, &log_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "log2") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "log2", 0, &log2_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "log10") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "log10", 0, &log10_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "exp") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "exp", 0, &exp_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "random") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "random", 0, &random_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 19);
}
