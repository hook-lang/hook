//
// math.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "math.h"
#include <math.h>

static void abs_call(HkVM *vm, HkValue *args);
static void sin_call(HkVM *vm, HkValue *args);
static void cos_call(HkVM *vm, HkValue *args);
static void tan_call(HkVM *vm, HkValue *args);
static void asin_call(HkVM *vm, HkValue *args);
static void acos_call(HkVM *vm, HkValue *args);
static void atan_call(HkVM *vm, HkValue *args);
static void floor_call(HkVM *vm, HkValue *args);
static void ceil_call(HkVM *vm, HkValue *args);
static void round_call(HkVM *vm, HkValue *args);
static void pow_call(HkVM *vm, HkValue *args);
static void sqrt_call(HkVM *vm, HkValue *args);
static void cbrt_call(HkVM *vm, HkValue *args);
static void log_call(HkVM *vm, HkValue *args);
static void log2_call(HkVM *vm, HkValue *args);
static void log10_call(HkVM *vm, HkValue *args);
static void exp_call(HkVM *vm, HkValue *args);

static void abs_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, fabs(hk_as_number(args[1])));
}

static void sin_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, sin(hk_as_number(args[1])));
}

static void cos_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, cos(hk_as_number(args[1])));
}

static void tan_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, tan(hk_as_number(args[1])));
}

static void asin_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, asin(hk_as_number(args[1])));
}

static void acos_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, acos(hk_as_number(args[1])));
}

static void atan_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, atan(hk_as_number(args[1])));
}

static void floor_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, floor(hk_as_number(args[1])));
}

static void ceil_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, ceil(hk_as_number(args[1])));
}

static void round_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, round(hk_as_number(args[1])));
}

static void pow_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, pow(hk_as_number(args[1]), hk_as_number(args[2])));
}

static void sqrt_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, sqrt(hk_as_number(args[1])));
}

static void cbrt_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, cbrt(hk_as_number(args[1])));
}

static void log_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, log(hk_as_number(args[1])));
}

static void log2_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, log2(hk_as_number(args[1])));
}

static void log10_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, log10(hk_as_number(args[1])));
}

static void exp_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, exp(hk_as_number(args[1])));
}

HK_LOAD_MODULE_HANDLER(math)
{
  hk_vm_push_string_from_chars(vm, -1, "math");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "abs");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "abs", 1, abs_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "sin");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "sin", 1, sin_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "cos");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "cos", 1, cos_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "tan");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "tan", 1, tan_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "asin");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "asin", 1, asin_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "acos");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "acos", 1, acos_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "atan");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "atan", 1, atan_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "floor");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "floor", 1, floor_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "ceil");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "ceil", 1, ceil_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "round");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "round", 1, round_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "pow");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "pow", 2, pow_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "sqrt");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "sqrt", 1, sqrt_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "cbrt");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "cbrt", 1, cbrt_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "log");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "log", 0, log_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "log2");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "log2", 0, log2_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "log10");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "log10", 0, log10_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "exp");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "exp", 0, exp_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 17);
}
