//
// strings.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "strings.h"
#include <string.h>

static void new_string_call(HkVM *vm, HkValue *args);
static void repeat_call(HkVM *vm, HkValue *args);
static void hash_call(HkVM *vm, HkValue *args);
static void lower_call(HkVM *vm, HkValue *args);
static void upper_call(HkVM *vm, HkValue *args);
static void trim_call(HkVM *vm, HkValue *args);
static void starts_with_call(HkVM *vm, HkValue *args);
static void ends_with_call(HkVM *vm, HkValue *args);
static void reverse_call(HkVM *vm, HkValue *args);

static void new_string_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_int(vm, args, 1);
  hk_return_if_not_ok(vm);
  int capacity = (int) hk_as_number(args[1]);
  HkString *str = hk_string_new_with_capacity(capacity);
  hk_vm_push_string(vm, str);
  if (!hk_vm_is_ok(vm))
    hk_string_free(str);
}

static void repeat_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  int count = (int) hk_as_number(args[2]);
  count = count < 0 ? 0 : count;
  int length = str->length;
  int new_length = length * count;
  HkString *result = hk_string_new_with_capacity(new_length);
  char *src = str->chars;
  char *dest = result->chars;
  for (int i = 0; i < count; ++i)
  {
    memcpy(dest, src, length);
    dest += length;
  }
  result->length = new_length;
  result->chars[new_length] = '\0';
  hk_vm_push_string(vm, result);
  if (!hk_vm_is_ok(vm))
    hk_string_free(result);
}

static void hash_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, hk_string_hash(hk_as_string(args[1])));
}

static void lower_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_string_lower(hk_as_string(args[1]));
  hk_vm_push_string(vm, str);
  if (!hk_vm_is_ok(vm))
    hk_string_free(str);
}

static void upper_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_string_upper(hk_as_string(args[1]));
  hk_vm_push_string(vm, str);
  if (!hk_vm_is_ok(vm))
    hk_string_free(str);
}

static void trim_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str;
  if (!hk_string_trim(hk_as_string(args[1]), &str))
    return;
  hk_vm_push_string(vm, str);
  if (!hk_vm_is_ok(vm))
    hk_string_free(str);
}

static void starts_with_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_push_bool(vm, hk_string_starts_with(hk_as_string(args[1]), hk_as_string(args[2])));
}

static void ends_with_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_push_bool(vm, hk_string_ends_with(hk_as_string(args[1]), hk_as_string(args[2])));
}

static void reverse_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_string_reverse(hk_as_string(args[1]));
  hk_vm_push_string(vm, str);
  if (!hk_vm_is_ok(vm))
    hk_string_free(str);
}

HK_LOAD_MODULE_HANDLER(strings)
{
  hk_vm_push_string_from_chars(vm, -1, "strings");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "new_string");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "new_string", 1, new_string_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "repeat");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "repeat", 2, repeat_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "hash");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "hash", 1, hash_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "lower");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "lower", 1, lower_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "upper");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "upper", 1, upper_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "trim");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "trim", 1, trim_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "starts_with");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "starts_with", 2, starts_with_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "ends_with");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "ends_with", 2, ends_with_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "reverse");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "reverse", 1, reverse_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 9);
}
