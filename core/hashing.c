//
// The Hook Programming Language
// hashing.c
//

#include "hashing.h"
#include <string.h>
#include <hook/check.h>
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
static void crc32_call(HkState *state, HkValue *args);
static void crc64_call(HkState *state, HkValue *args);
static void sha224_call(HkState *state, HkValue *args);
static void sha256_call(HkState *state, HkValue *args);
static void sha384_call(HkState *state, HkValue *args);
static void sha512_call(HkState *state, HkValue *args);
static void sha1_call(HkState *state, HkValue *args);
static void sha3_call(HkState *state, HkValue *args);
static void md5_call(HkState *state, HkValue *args);
static void ripemd160_call(HkState *state, HkValue *args);

static inline void md5(char *chars, int length, char *result)
{
  MD5Context ctx;
  md5Init(&ctx);
  md5Update(&ctx, (uint8_t *) chars, length);
  md5Finalize(&ctx);
  memcpy(result, ctx.digest, MD5_DIGEST_SIZE);
}

static void crc32_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  uint32_t result = crc32(str->chars, str->length);
  hk_state_push_number(state, (double) result);
}

static void crc64_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  uint64_t result = crc64(str->chars, str->length);
  hk_state_push_number(state, (double) result);
}

static void sha224_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  int length = SHA224_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  sha224((unsigned char *) str->chars, str->length, (unsigned char *) result->chars);
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void sha256_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  int length = SHA256_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  sha256((unsigned char *) str->chars, str->length, (unsigned char *) result->chars);
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void sha384_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  int length = SHA384_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  sha384((unsigned char *) str->chars, str->length, (unsigned char *) result->chars);
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void sha512_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  int length = SHA512_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  sha512((unsigned char *) str->chars, str->length, (unsigned char *) result->chars);
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void sha1_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  Sha1Digest digest = Sha1_get(str->chars, str->length);
  int length = SHA1_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  memcpy(result->chars, digest.digest, length);
  result->length = length;
  result->chars[length] = '\0';
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void sha3_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  int length = SHA3_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  sha3(str->chars, str->length, result->chars, length);
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void md5_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  int length = MD5_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  md5(str->chars, str->length, result->chars);
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void ripemd160_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  int length = RIPEMD160_DIGEST_SIZE;
  HkString *result = hk_string_new_with_capacity(length);
  result->length = length;
  result->chars[length] = '\0';
  ripemd160((uint8_t *) str->chars, str->length, (uint8_t *) result->chars);
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

HK_LOAD_MODULE_HANDLER(hashing)
{
  hk_state_push_string_from_chars(state, -1, "hashing");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "crc32");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "crc32", 1, &crc32_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "crc64");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "crc64", 1, &crc64_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sha224");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sha224", 1, &sha224_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sha256");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sha256", 1, &sha256_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sha384");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sha384", 1, &sha384_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sha512");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sha512", 1, &sha512_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sha1");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sha1", 1, &sha1_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sha3");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sha3", 1, &sha3_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "md5");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "md5", 1, &md5_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "ripemd160");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "ripemd160", 1, &ripemd160_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 10);
}
