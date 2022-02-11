//
// Hook Programming Language
// hashing.c
//

#include "hashing.h"
#include "sha2.h"
#include "ripemd160.h"
#include "common.h"

#define RIPEMD160_DIGEST_SIZE 20

static int sha224_call(vm_t *vm, value_t *args);
static int sha256_call(vm_t *vm, value_t *args);
static int sha384_call(vm_t *vm, value_t *args);
static int sha512_call(vm_t *vm, value_t *args);
static int ripemd160_call(vm_t *vm, value_t *args);

static int sha224_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  int length = SHA224_DIGEST_SIZE;
  string_t *digest = string_allocate(length);
  digest->length = length;
  digest->chars[length] = '\0';
  sha224((unsigned char *) str->chars, str->length, (unsigned char *) digest->chars);
  if (vm_push_string(vm, digest) == STATUS_ERROR)
  {
    string_free(digest);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int sha256_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  int length = SHA256_DIGEST_SIZE;
  string_t *digest = string_allocate(length);
  digest->length = length;
  digest->chars[length] = '\0';
  sha256((unsigned char *) str->chars, str->length, (unsigned char *) digest->chars);
  if (vm_push_string(vm, digest) == STATUS_ERROR)
  {
    string_free(digest);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int sha384_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  int length = SHA384_DIGEST_SIZE;
  string_t *digest = string_allocate(length);
  digest->length = length;
  digest->chars[length] = '\0';
  sha384((unsigned char *) str->chars, str->length, (unsigned char *) digest->chars);
  if (vm_push_string(vm, digest) == STATUS_ERROR)
  {
    string_free(digest);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int sha512_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  int length = SHA512_DIGEST_SIZE;
  string_t *digest = string_allocate(length);
  digest->length = length;
  digest->chars[length] = '\0';
  sha512((unsigned char *) str->chars, str->length, (unsigned char *) digest->chars);
  if (vm_push_string(vm, digest) == STATUS_ERROR)
  {
    string_free(digest);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int ripemd160_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *str = AS_STRING(args[1]);
  int length = RIPEMD160_DIGEST_SIZE;
  string_t *digest = string_allocate(length);
  digest->length = length;
  digest->chars[length] = '\0';
  ripemd160((uint8_t *) str->chars, str->length, (uint8_t *) digest->chars);
  if (vm_push_string(vm, digest) == STATUS_ERROR)
  {
    string_free(digest);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_hashing(vm_t *vm)
#else
int load_hashing(vm_t *vm)
#endif
{
  if (vm_push_string_from_chars(vm, -1, "hashing") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "sha224") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "sha224", 1, &sha224_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "sha256") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "sha256", 1, &sha256_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "sha384") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "sha384", 1, &sha384_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "sha512") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "sha512", 1, &sha512_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "ripemd160") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "ripemd160", 1, &ripemd160_call) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_construct(vm, 5);
}
