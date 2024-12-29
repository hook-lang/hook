//
// utf8.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "utf8.h"

static inline int decode_char(unsigned char c);
static void len_call(HkVM *vm, HkValue *args);
static void sub_call(HkVM *vm, HkValue *args);

static inline int decode_char(unsigned char c)
{
  if ((c & 0xc0) == 0x80)
    return 0;
  if ((c & 0xf8) == 0xf0)
    return 4;
  if ((c & 0xf0) == 0xe0)
    return 3;
  if ((c & 0xe0) == 0xc0)
    return 2;
  return 1;
}

static void len_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  int result = 0;
  for (int i = 0; i < str->length;)
  {
    int length = decode_char((unsigned char) str->chars[i]);
    if (!length)
      break;
    i += length;
    ++result;
  }
  hk_vm_push_number(vm, result);
}

static void sub_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 3);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  int start = (int) hk_as_number(args[2]);
  int end = (int) hk_as_number(args[3]);
  int length = 0;
  int i = 0;
  while (i < str->length)
  {
    int n = decode_char((unsigned char) str->chars[i]);
    if (!n || length == start)
      break;
    i += n;
    ++length;
  }
  start = i;
  while (i < str->length)
  {
    int n = decode_char((unsigned char) str->chars[i]);
    if (!n || length == end)
      break;
    i += n;
    ++length;
  }
  end = i;
  length = end - start;
  char *chars = &str->chars[start];
  hk_vm_push_string_from_chars(vm, length, chars);
}

HK_LOAD_MODULE_HANDLER(utf8)
{
  hk_vm_push_string_from_chars(vm, -1, "utf8");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "len");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "len", 1, len_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "sub");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "sub", 3, sub_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 2);
}
