//
// The Hook Programming Language
// math.c
//

#include "math.h"
#include <math.h>
#include <hook/check.h>
#include <hook/status.h>

static int abs_call(HkState *state, HkValue *args);
static int sin_call(HkState *state, HkValue *args);
static int cos_call(HkState *state, HkValue *args);
static int tan_call(HkState *state, HkValue *args);
static int asin_call(HkState *state, HkValue *args);
static int acos_call(HkState *state, HkValue *args);
static int atan_call(HkState *state, HkValue *args);
static int floor_call(HkState *state, HkValue *args);
static int ceil_call(HkState *state, HkValue *args);
static int round_call(HkState *state, HkValue *args);
static int pow_call(HkState *state, HkValue *args);
static int sqrt_call(HkState *state, HkValue *args);
static int cbrt_call(HkState *state, HkValue *args);
static int log_call(HkState *state, HkValue *args);
static int log2_call(HkState *state, HkValue *args);
static int log10_call(HkState *state, HkValue *args);
static int exp_call(HkState *state, HkValue *args);

static int abs_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, fabs(hk_as_number(args[1])));
}

static int sin_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, sin(hk_as_number(args[1])));
}

static int cos_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, cos(hk_as_number(args[1])));
}

static int tan_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, tan(hk_as_number(args[1])));
}

static int asin_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, asin(hk_as_number(args[1])));
}

static int acos_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, acos(hk_as_number(args[1])));
}

static int atan_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, atan(hk_as_number(args[1])));
}

static int floor_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, floor(hk_as_number(args[1])));
}

static int ceil_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, ceil(hk_as_number(args[1])));
}

static int round_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, round(hk_as_number(args[1])));
}

static int pow_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_number(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, pow(hk_as_number(args[1]), hk_as_number(args[2])));
}

static int sqrt_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, sqrt(hk_as_number(args[1])));
}

static int cbrt_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, cbrt(hk_as_number(args[1])));
}

static int log_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, log(hk_as_number(args[1])));
}

static int log2_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, log2(hk_as_number(args[1])));
}

static int log10_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, log10(hk_as_number(args[1])));
}

static int exp_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_number(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, exp(hk_as_number(args[1])));
}

HK_LOAD_FN(math)
{
  if (hk_state_push_string_from_chars(state, -1, "math") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "abs") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "abs", 1, &abs_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sin") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sin", 1, &sin_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "cos") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "cos", 1, &cos_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "tan") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "tan", 1, &tan_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "asin") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "asin", 1, &asin_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "acos") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "acos", 1, &acos_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "atan") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "atan", 1, &atan_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "floor") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "floor", 1, &floor_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "ceil") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "ceil", 1, &ceil_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "round") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "round", 1, &round_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "pow") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "pow", 2, &pow_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sqrt") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sqrt", 1, &sqrt_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "cbrt") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "cbrt", 1, &cbrt_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "log") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "log", 0, &log_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "log2") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "log2", 0, &log2_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "log10") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "log10", 0, &log10_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "exp") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "exp", 0, &exp_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 17);
}
