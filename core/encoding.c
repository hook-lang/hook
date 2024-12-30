//
// encoding.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "encoding.h"
#include "deps/ascii85.h"
#include "deps/base32.h"
#include "deps/base64.h"
#include "deps/base58.h"

#define BASE58_ENCODE_OUT_SIZE(n) ((n) * 138 / 100 + 1)
#define BASE58_DECODE_OUT_SIZE(n) ((n) * 733 / 1000 + 1)

static void base32_encode_call(HkVM *vm, HkValue *args);
static void base32_decode_call(HkVM *vm, HkValue *args);
static void base58_encode_call(HkVM *vm, HkValue *args);
static void base58_decode_call(HkVM *vm, HkValue *args);
static void base64_encode_call(HkVM *vm, HkValue *args);
static void base64_decode_call(HkVM *vm, HkValue *args);
static void ascii85_encode_call(HkVM *vm, HkValue *args);
static void ascii85_decode_call(HkVM *vm, HkValue *args);

static void base32_encode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  int length = BASE32_LEN(str->length);
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[result->length] = '\0';
  base32_encode((unsigned char *) str->chars, str->length, (unsigned char *) result->chars);
  hk_vm_push_string(vm, result);
  if (!hk_vm_is_ok(vm))
    hk_string_free(result);
}

static void base32_decode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  HkString *result = hk_string_new_with_capacity(UNBASE32_LEN(str->length));
  int length = (int) base32_decode((unsigned char *) str->chars,
    (unsigned char *) result->chars);
  result->length = length;
  result->chars[length] = '\0';
  hk_vm_push_string(vm, result);
  if (!hk_vm_is_ok(vm))
    hk_string_free(result);
}

static void base58_encode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  HkString *result = hk_string_new_with_capacity(BASE58_ENCODE_OUT_SIZE(str->length));
  size_t out_len = result->capacity;
  (void) base58_encode(str->chars, str->length, result->chars, &out_len);
  result->length = (int) out_len;
  result->chars[result->length] = '\0';
  hk_vm_push_string(vm, result);
  if (!hk_vm_is_ok(vm))
    hk_string_free(result);
}

static void base58_decode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  HkString *result = hk_string_new_with_capacity(BASE58_DECODE_OUT_SIZE(str->length));
  size_t out_len = result->capacity;
  (void) base58_decode(str->chars, str->length, result->chars, &out_len);
  result->length = (int) out_len;
  result->chars[result->length] = '\0';
  hk_vm_push_string(vm, result);
  if (!hk_vm_is_ok(vm))
    hk_string_free(result);
}

static void base64_encode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  int length = BASE64_ENCODE_OUT_SIZE(str->length) - 1;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  (void) base64_encode((unsigned char *) str->chars, str->length, result->chars);
  hk_vm_push_string(vm, result);
  if (!hk_vm_is_ok(vm))
    hk_string_free(result);
}

static void base64_decode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  int length = BASE64_DECODE_OUT_SIZE(str->length) - 1;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  (void) base64_decode(str->chars, str->length, (unsigned char *) result->chars);
  hk_vm_push_string(vm, result);
  if (!hk_vm_is_ok(vm))
    hk_string_free(result);
}

static void ascii85_encode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  int max_length = ascii85_get_max_encoded_length(str->length);
  HkString *result = hk_string_new_with_capacity(max_length);
  int length = encode_ascii85((const uint8_t *) str->chars, str->length, (uint8_t *) result->chars, max_length);
  result->length = length;
  result->chars[length] = '\0';
  hk_vm_push_string(vm, result);
  if (!hk_vm_is_ok(vm))
    hk_string_free(result);
}

static void ascii85_decode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  int max_length = ascii85_get_max_decoded_length(str->length);
  HkString *result = hk_string_new_with_capacity(max_length);
  int length = decode_ascii85((const uint8_t *) str->chars, str->length, (uint8_t *) result->chars, max_length);
  result->length = length;
  result->chars[length] = '\0';
  hk_vm_push_string(vm, result);
  if (!hk_vm_is_ok(vm))
    hk_string_free(result);
}

HK_LOAD_MODULE_HANDLER(encoding)
{
  hk_vm_push_string_from_chars(vm, -1, "encoding");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "base32_encode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "base32_encode", 1, base32_encode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "base32_decode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "base32_decode", 1, base32_decode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "base58_encode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "base58_encode", 1, base58_encode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "base58_decode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "base58_decode", 1, base58_decode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "base64_encode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "base64_encode", 1, base64_encode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "base64_decode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "base64_decode", 1, base64_decode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "ascii85_encode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "ascii85_encode", 1, ascii85_encode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "ascii85_decode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "ascii85_decode", 1, ascii85_decode_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 8);
}
