//
// The Hook Programming Language
// math.c
//

#include "math.h"
#include <math.h>
#include <hook/check.h>

static void abs_call(HkState *state, HkValue *args);
static void sin_call(HkState *state, HkValue *args);
static void cos_call(HkState *state, HkValue *args);
static void tan_call(HkState *state, HkValue *args);
static void asin_call(HkState *state, HkValue *args);
static void acos_call(HkState *state, HkValue *args);
static void atan_call(HkState *state, HkValue *args);
static void floor_call(HkState *state, HkValue *args);
static void ceil_call(HkState *state, HkValue *args);
static void round_call(HkState *state, HkValue *args);
static void pow_call(HkState *state, HkValue *args);
static void sqrt_call(HkState *state, HkValue *args);
static void cbrt_call(HkState *state, HkValue *args);
static void log_call(HkState *state, HkValue *args);
static void log2_call(HkState *state, HkValue *args);
static void log10_call(HkState *state, HkValue *args);
static void exp_call(HkState *state, HkValue *args);

static void abs_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, fabs(hk_as_number(args[1])));
}

static void sin_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, sin(hk_as_number(args[1])));
}

static void cos_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, cos(hk_as_number(args[1])));
}

static void tan_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, tan(hk_as_number(args[1])));
}

static void asin_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, asin(hk_as_number(args[1])));
}

static void acos_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, acos(hk_as_number(args[1])));
}

static void atan_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, atan(hk_as_number(args[1])));
}

static void floor_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, floor(hk_as_number(args[1])));
}

static void ceil_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, ceil(hk_as_number(args[1])));
}

static void round_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, round(hk_as_number(args[1])));
}

static void pow_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_number(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, pow(hk_as_number(args[1]), hk_as_number(args[2])));
}

static void sqrt_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, sqrt(hk_as_number(args[1])));
}

static void cbrt_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, cbrt(hk_as_number(args[1])));
}

static void log_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, log(hk_as_number(args[1])));
}

static void log2_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, log2(hk_as_number(args[1])));
}

static void log10_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, log10(hk_as_number(args[1])));
}

static void exp_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, exp(hk_as_number(args[1])));
}

HK_LOAD_MODULE_HANDLER(math)
{
  hk_state_push_string_from_chars(state, -1, "math");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "abs");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "abs", 1, &abs_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sin");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sin", 1, &sin_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "cos");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "cos", 1, &cos_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "tan");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "tan", 1, &tan_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "asin");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "asin", 1, &asin_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "acos");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "acos", 1, &acos_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "atan");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "atan", 1, &atan_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "floor");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "floor", 1, &floor_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "ceil");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "ceil", 1, &ceil_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "round");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "round", 1, &round_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "pow");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "pow", 2, &pow_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sqrt");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sqrt", 1, &sqrt_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "cbrt");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "cbrt", 1, &cbrt_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "log");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "log", 0, &log_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "log2");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "log2", 0, &log2_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "log10");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "log10", 0, &log10_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "exp");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "exp", 0, &exp_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 17);
}
