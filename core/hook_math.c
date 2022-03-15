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

static int abs_call(hk_vm_t *vm, hk_value_t *args);
static int sin_call(hk_vm_t *vm, hk_value_t *args);
static int cos_call(hk_vm_t *vm, hk_value_t *args);
static int tan_call(hk_vm_t *vm, hk_value_t *args);
static int asin_call(hk_vm_t *vm, hk_value_t *args);
static int acos_call(hk_vm_t *vm, hk_value_t *args);
static int atan_call(hk_vm_t *vm, hk_value_t *args);
static int floor_call(hk_vm_t *vm, hk_value_t *args);
static int ceil_call(hk_vm_t *vm, hk_value_t *args);
static int pow_call(hk_vm_t *vm, hk_value_t *args);
static int sqrt_call(hk_vm_t *vm, hk_value_t *args);
static int log_call(hk_vm_t *vm, hk_value_t *args);
static int exp_call(hk_vm_t *vm, hk_value_t *args);
static int random_call(hk_vm_t *vm, hk_value_t *args);

static int abs_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, fabs(args[1].as.number));
}

static int sin_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, sin(args[1].as.number));
}

static int cos_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, cos(args[1].as.number));
}

static int tan_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, tan(args[1].as.number));
}

static int asin_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, asin(args[1].as.number));
}

static int acos_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, acos(args[1].as.number));
}

static int atan_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, atan(args[1].as.number));
}

static int floor_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, floor(args[1].as.number));
}

static int ceil_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, ceil(args[1].as.number));
}

static int pow_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_number(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, pow(args[1].as.number, args[2].as.number));
}

static int sqrt_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, sqrt(args[1].as.number));
}

static int log_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, log(args[1].as.number));
}

static int exp_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, exp(args[1].as.number));
}

static int random_call(hk_vm_t *vm, hk_value_t *args)
{
  (void) args;
  double result = (double) rand() / RAND_MAX;
  return hk_vm_push_number(vm, result);
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_math(hk_vm_t *vm)
#else
int load_math(hk_vm_t *vm)
#endif
{
  srand((unsigned) time(NULL));
  if (hk_vm_push_string_from_chars(vm, -1, "math") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "Pi") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_number(vm, M_PI) == HK_STATUS_ERROR)
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
  if (hk_vm_push_string_from_chars(vm, -1, "pow") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "pow", 2, &pow_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "sqrt") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sqrt", 1, &sqrt_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "log") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "log", 0, &log_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "exp") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "exp", 0, &exp_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "random") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "random", 0, &random_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 15);
}
