//
// geohash.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "geohash.h"
#include "deps/geohash.h"

static void encode_call(HkVM *vm, HkValue *args);
static void decode_call(HkVM *vm, HkValue *args);

static void encode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_number(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_number(vm, args, 2);
  hk_return_if_not_ok(vm);
  double lat = hk_as_number(args[1]);
  double lon = hk_as_number(args[2]);
  char buf[16] = {0};
  if (geohash_encode(lat, lon, buf, sizeof(buf)) != GEOHASH_OK)
  {
    hk_vm_push_nil(vm);
    return;
  }
  HkString *str = hk_string_from_chars(-1, buf);
  hk_vm_push_string(vm, str);
  if (!hk_vm_is_ok(vm))
    hk_string_free(str);
}

static void decode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  double lat = 0.0;
  double lon = 0.0;
  if (geohash_decode(str->chars, &lat, &lon) != GEOHASH_OK)
  {
    hk_vm_push_nil(vm);
    return;
  }
  HkArray *arr = hk_array_new_with_capacity(2);
  hk_array_inplace_append_element(arr, hk_number_value(lat));
  hk_array_inplace_append_element(arr, hk_number_value(lon));
  hk_vm_push_array(vm, arr);
  if (!hk_vm_is_ok(vm))
    hk_array_free(arr);
}

HK_LOAD_MODULE_HANDLER(geohash)
{
  hk_vm_push_string_from_chars(vm, -1, "geohash");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "encode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "encode", 2, encode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "decode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "decode", 1, decode_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 2);
}
