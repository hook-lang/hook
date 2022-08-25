//
// The Hook Programming Language
// hashing.c
//

#include "hashing.h"
#include "deps/sha2.h"
#include "deps/ripemd160.h"
#include "hk_status.h"

#define RIPEMD160_DIGEST_SIZE 20

static int32_t sha224_call(hk_vm_t *vm, hk_value_t *args);
static int32_t sha256_call(hk_vm_t *vm, hk_value_t *args);
static int32_t sha384_call(hk_vm_t *vm, hk_value_t *args);
static int32_t sha512_call(hk_vm_t *vm, hk_value_t *args);
static int32_t ripemd160_call(hk_vm_t *vm, hk_value_t *args);

static int32_t sha224_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  int32_t length = SHA224_DIGEST_SIZE;
  hk_string_t *digest = hk_string_new_with_capacity(length);
  digest->length = length;
  digest->chars[length] = '\0';
  sha224((unsigned char *) str->chars, str->length, (unsigned char *) digest->chars);
  if (hk_vm_push_string(vm, digest) == HK_STATUS_ERROR)
  {
    hk_string_free(digest);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t sha256_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  int32_t length = SHA256_DIGEST_SIZE;
  hk_string_t *digest = hk_string_new_with_capacity(length);
  digest->length = length;
  digest->chars[length] = '\0';
  sha256((unsigned char *) str->chars, str->length, (unsigned char *) digest->chars);
  if (hk_vm_push_string(vm, digest) == HK_STATUS_ERROR)
  {
    hk_string_free(digest);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t sha384_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  int32_t length = SHA384_DIGEST_SIZE;
  hk_string_t *digest = hk_string_new_with_capacity(length);
  digest->length = length;
  digest->chars[length] = '\0';
  sha384((unsigned char *) str->chars, str->length, (unsigned char *) digest->chars);
  if (hk_vm_push_string(vm, digest) == HK_STATUS_ERROR)
  {
    hk_string_free(digest);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t sha512_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  int32_t length = SHA512_DIGEST_SIZE;
  hk_string_t *digest = hk_string_new_with_capacity(length);
  digest->length = length;
  digest->chars[length] = '\0';
  sha512((unsigned char *) str->chars, str->length, (unsigned char *) digest->chars);
  if (hk_vm_push_string(vm, digest) == HK_STATUS_ERROR)
  {
    hk_string_free(digest);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t ripemd160_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *str = hk_as_string(args[1]);
  int32_t length = RIPEMD160_DIGEST_SIZE;
  hk_string_t *digest = hk_string_new_with_capacity(length);
  digest->length = length;
  digest->chars[length] = '\0';
  ripemd160((uint8_t *) str->chars, str->length, (uint8_t *) digest->chars);
  if (hk_vm_push_string(vm, digest) == HK_STATUS_ERROR)
  {
    hk_string_free(digest);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_hashing(hk_vm_t *vm)
#else
int32_t load_hashing(hk_vm_t *vm)
#endif
{
  if (hk_vm_push_string_from_chars(vm, -1, "hashing") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "sha224") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sha224", 1, &sha224_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "sha256") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sha256", 1, &sha256_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "sha384") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sha384", 1, &sha384_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "sha512") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sha512", 1, &sha512_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "ripemd160") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "ripemd160", 1, &ripemd160_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 5);
}
