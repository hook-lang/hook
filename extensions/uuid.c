//
// uuid.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "uuid.h"
#include "deps/uuid4.h"

static void random_call(HkVM *vm, HkValue *args);

static void random_call(HkVM *vm, HkValue *args)
{
  (void) args;
  HkString *str = hk_string_new_with_capacity(UUID4_LEN);
  uuid4_init();
  uuid4_generate(str->chars);
  str->length = UUID4_LEN;
  str->chars[str->length] = '\0';
  hk_vm_push_string(vm, str);
}

HK_LOAD_MODULE_HANDLER(uuid)
{
  hk_vm_push_string_from_chars(vm, -1, "uuid");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "random");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "random", 0, random_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 1);
}
