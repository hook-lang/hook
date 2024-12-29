//
// numbers.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "numbers.h"
#include <stdlib.h>
#include <float.h>

#define PI  3.14159265358979323846264338327950288
#define TAU 6.28318530717958647692528676655900577

#define LARGEST  DBL_MAX
#define SMALLEST DBL_MIN

#define MAX_INTEGER 9007199254740991.0
#define MIN_INTEGER -9007199254740991.0

static void srand_call(HkVM *vm, HkValue *args);
static void rand_call(HkVM *vm, HkValue *args);

static void srand_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  srand((uint32_t) hk_as_number(args[1]));
  hk_vm_push_nil(vm);
}

static void rand_call(HkVM *vm, HkValue *args)
{
  (void) args;
  double result = (double) rand() / RAND_MAX;
  hk_vm_push_number(vm, result);
}

HK_LOAD_MODULE_HANDLER(numbers)
{
  hk_vm_push_string_from_chars(vm, -1, "numbers");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "PI");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, PI);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "TAU");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, TAU);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "LARGEST");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, LARGEST);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SMALLEST");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, SMALLEST);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "MAX_INTEGER");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, MAX_INTEGER);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "MIN_INTEGER");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, MIN_INTEGER);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "srand");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "srand", 1, srand_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "rand");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "rand", 0, rand_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 8);
}
