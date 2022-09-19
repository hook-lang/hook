//
// The Hook Programming Language
// encoding.c
//

#include "encoding.h"
#include "deps/base32.h"
#include "deps/base64.h"
#include "deps/base58.h"
#include "hk_status.h"

#define BASE58_ENCODE_OUT_SIZE(n) ((n) * 138 / 100 + 1)
#define BASE58_DECODE_OUT_SIZE(n) ((n) * 733 /1000 + 1)

static int32_t base32_encode_call(hk_vm_t *vm, hk_value_t *args);
static int32_t base32_decode_call(hk_vm_t *vm, hk_value_t *args);
static int32_t base58_encode_call(hk_vm_t *vm, hk_value_t *args);
static int32_t base58_decode_call(hk_vm_t *vm, hk_value_t *args);
static int32_t base64_encode_call(hk_vm_t *vm, hk_value_t *args);
static int32_t base64_decode_call(hk_vm_t *vm, hk_value_t *args);

static int32_t base32_encode_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  int32_t length = BASE32_LEN(str->length);
  hk_string_t *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[result->length] = '\0';
  base32_encode((unsigned char *) str->chars, str->length, (unsigned char *) result->chars);
  if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t base32_decode_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  hk_string_t *result = hk_string_new_with_capacity(UNBASE32_LEN(str->length));
  int32_t length = (int32_t) base32_decode((unsigned char *) str->chars,
    (unsigned char *) result->chars);
  result->length = length;
  result->chars[length] = '\0';
  if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t base58_encode_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  hk_string_t *result = hk_string_new_with_capacity(BASE58_ENCODE_OUT_SIZE(str->length));
  size_t out_len;
  (void) base58_encode(str->chars, str->length, result->chars, &out_len);
  result->length = (int32_t) out_len;
  result->chars[result->length] = '\0';
  if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t base58_decode_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  hk_string_t *result = hk_string_new_with_capacity(BASE58_DECODE_OUT_SIZE(str->length));
  size_t out_len;
  (void) base58_decode(str->chars, str->length, result->chars, &out_len);
  result->length = (int32_t) out_len;
  result->chars[result->length] = '\0';
  if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t base64_encode_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  int32_t length = BASE64_ENCODE_OUT_SIZE(str->length) - 1;
  hk_string_t *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  (void) base64_encode((unsigned char *) str->chars, str->length, result->chars);
  if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t base64_decode_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  int32_t length = BASE64_DECODE_OUT_SIZE(str->length) - 1;
  hk_string_t *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  (void) base64_decode(str->chars, str->length, (unsigned char *) result->chars);
  if (hk_vm_push_string(vm, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

HK_LOAD_FN(encoding)
{
  if (hk_vm_push_string_from_chars(vm, -1, "encoding") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "base32_encode") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "base32_encode", 1, &base32_encode_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "base32_decode") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "base32_decode", 1, &base32_decode_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "base58_encode") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "base58_encode", 1, &base58_encode_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "base58_decode") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "base58_decode", 1, &base58_decode_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "base64_encode") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "base64_encode", 1, &base64_encode_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "base64_decode") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "base64_decode", 1, &base64_decode_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 6);
}
