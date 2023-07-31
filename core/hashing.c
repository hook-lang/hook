//
// The Hook Programming Language
// hashing.c
//

#include "hashing.h"
#include <hook/check.h>
#include <hook/status.h>
#include <string.h>
#include "deps/crc32.h"
#include "deps/crc64.h"
#include "deps/sha1.h"
#include "deps/sha2.h"
#include "deps/sha3.h"
#include "deps/md5.h"
#include "deps/ripemd160.h"

#define SHA1_DIGEST_SIZE      20
#define SHA3_DIGEST_SIZE      32
#define MD5_DIGEST_SIZE       16
#define RIPEMD160_DIGEST_SIZE 20

static inline void md5(char *chars, int length, char *result);
static int crc32_call(HkState *state, HkValue *args);
static int crc64_call(HkState *state, HkValue *args);
static int sha224_call(HkState *state, HkValue *args);
static int sha256_call(HkState *state, HkValue *args);
static int sha384_call(HkState *state, HkValue *args);
static int sha512_call(HkState *state, HkValue *args);
static int sha1_call(HkState *state, HkValue *args);
static int sha3_call(HkState *state, HkValue *args);
static int md5_call(HkState *state, HkValue *args);
static int ripemd160_call(HkState *state, HkValue *args);

static inline void md5(char *chars, int length, char *result)
{
  MD5Context ctx;
  md5Init(&ctx);
  md5Update(&ctx, (uint8_t *) chars, length);
  md5Finalize(&ctx);
  memcpy(result, ctx.digest, MD5_DIGEST_SIZE);
}

static int crc32_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  uint32_t result = crc32(str->chars, str->length);
  if (hk_state_push_number(state, (double) result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return HK_STATUS_OK;
}

static int crc64_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  uint64_t result = crc64(str->chars, str->length);
  if (hk_state_push_number(state, (double) result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return HK_STATUS_OK;
}

static int sha224_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  int length = SHA224_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  sha224((unsigned char *) str->chars, str->length, (unsigned char *) result->chars);
  if (hk_state_push_string(state, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int sha256_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  int length = SHA256_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  sha256((unsigned char *) str->chars, str->length, (unsigned char *) result->chars);
  if (hk_state_push_string(state, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int sha384_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  int length = SHA384_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  sha384((unsigned char *) str->chars, str->length, (unsigned char *) result->chars);
  if (hk_state_push_string(state, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int sha512_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  int length = SHA512_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  sha512((unsigned char *) str->chars, str->length, (unsigned char *) result->chars);
  if (hk_state_push_string(state, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int sha1_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  Sha1Digest digest = Sha1_get(str->chars, str->length);
  int length = SHA1_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  memcpy(result->chars, digest.digest, length);
  result->length = length;
  result->chars[length] = '\0';
  if (hk_state_push_string(state, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int sha3_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  int length = SHA3_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  sha3(str->chars, str->length, result->chars, length);
  if (hk_state_push_string(state, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int md5_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  int length = MD5_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  md5(str->chars, str->length, result->chars);
  if (hk_state_push_string(state, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int ripemd160_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  int length = RIPEMD160_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  ripemd160((uint8_t *) str->chars, str->length, (uint8_t *) result->chars);
  if (hk_state_push_string(state, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

HK_LOAD_FN(hashing)
{
  if (hk_state_push_string_from_chars(state, -1, "hashing") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "crc32") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "crc32", 1, &crc32_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "crc64") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "crc64", 1, &crc64_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sha224") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sha224", 1, &sha224_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sha256") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sha256", 1, &sha256_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sha384") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sha384", 1, &sha384_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sha512") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sha512", 1, &sha512_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sha1") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sha1", 1, &sha1_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "sha3") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "sha3", 1, &sha3_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "md5") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "md5", 1, &md5_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "ripemd160") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "ripemd160", 1, &ripemd160_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 10);
}
