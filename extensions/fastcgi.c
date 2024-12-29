//
// fastcgi.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "fastcgi.h"
#include <fcgi_stdio.h>

static void accept_call(HkVM *vm, HkValue *args);

static void accept_call(HkVM *vm, HkValue *args)
{
  (void) args;
  hk_vm_push_number(vm, FCGI_Accept());
}

HK_LOAD_MODULE_HANDLER(fastcgi)
{
  hk_vm_push_string_from_chars(vm, -1, "fastcgi");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "accept");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "accept", 0, accept_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 1);
}
