//
// Hook Programming Language
// encoding.c
//

#include "encoding.h"
#include "base32.h"
#include "base64.h"
#include "base58.h"
#include "h_common.h"

#define BASE58_ENCODE_OUT_SIZE(n) ((n) * 138 / 100 + 1)
#define BASE58_DECODE_OUT_SIZE(n) ((n) * 733 /1000 + 1)

static int base32_encode_call(vm_t *vm, value_t *args);
static int base32_decode_call(vm_t *vm, value_t *args);
static int base58_encode_call(vm_t *vm, value_t *args);
static int base58_decode_call(vm_t *vm, value_t *args);
static int base64_encode_call(vm_t *vm, value_t *args);
static int base64_decode_call(vm_t *vm, value_t *args);

static int base32_encode_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  int length = BASE32_LEN(str->length);
  string_t *result = string_allocate(length);
  result->length = length;
  result->chars[result->length] = '\0';
  base32_encode((unsigned char *) str->chars, str->length, (unsigned char *) result->chars);
  if (vm_push_string(vm, result) == STATUS_ERROR)
  {
    string_free(result);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int base32_decode_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  string_t *result = string_allocate(UNBASE32_LEN(str->length));
  int length = (int) base32_decode((unsigned char *) str->chars,
    (unsigned char *) result->chars);
  result->length = length;
  result->chars[length] = '\0';
  if (vm_push_string(vm, result) == STATUS_ERROR)
  {
    string_free(result);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int base58_encode_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  string_t *result = string_allocate(BASE58_ENCODE_OUT_SIZE(str->length));
  size_t out_len;
  (void) base58_encode(str->chars, str->length, result->chars, &out_len);
  result->length = (int) out_len;
  result->chars[result->length] = '\0';
  if (vm_push_string(vm, result) == STATUS_ERROR)
  {
    string_free(result);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int base58_decode_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  string_t *result = string_allocate(BASE58_DECODE_OUT_SIZE(str->length));
  size_t out_len;
  (void) base58_decode(str->chars, str->length, result->chars, &out_len);
  result->length = (int) out_len;
  result->chars[result->length] = '\0';
  if (vm_push_string(vm, result) == STATUS_ERROR)
  {
    string_free(result);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int base64_encode_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  int length = BASE64_ENCODE_OUT_SIZE(str->length) - 1;
  string_t *result = string_allocate(length);
  result->length = length;
  result->chars[length] = '\0';
  (void) base64_encode((unsigned char *) str->chars, str->length, result->chars);
  if (vm_push_string(vm, result) == STATUS_ERROR)
  {
    string_free(result);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int base64_decode_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  int length = BASE64_DECODE_OUT_SIZE(str->length) - 1;
  string_t *result = string_allocate(length);
  result->length = length;
  result->chars[length] = '\0';
  (void) base64_decode(str->chars, str->length, (unsigned char *) result->chars);
  if (vm_push_string(vm, result) == STATUS_ERROR)
  {
    string_free(result);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_encoding(vm_t *vm)
#else
int load_encoding(vm_t *vm)
#endif
{
  if (vm_push_string_from_chars(vm, -1, "encoding") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "base32_encode") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "base32_encode", 1, &base32_encode_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "base32_decode") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "base32_decode", 1, &base32_decode_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "base58_encode") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "base58_encode", 1, &base58_encode_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "base58_decode") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "base58_decode", 1, &base58_decode_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "base64_encode") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "base64_encode", 1, &base64_encode_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "base64_decode") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "base64_decode", 1, &base64_decode_call) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_construct(vm, 6);
}
