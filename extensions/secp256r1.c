//
// The Hook Programming Language
// secp256r1.c
//

#include "secp256r1.h"
#define ECC_CURVE 32
#include "deps/ecc.h"

#define PUBLIC_KEY_SIZE  (ECC_BYTES + 1)
#define PRIVATE_KEY_SIZE ECC_BYTES
#define SECRET_SIZE      ECC_BYTES
#define HASH_SIZE        ECC_BYTES
#define SIGNATURE_SIZE   (ECC_BYTES << 1)

static void new_key_pair_call(HkState *state, HkValue *args);
static void shared_secret_call(HkState *state, HkValue *args);
static void sign_hash_call(HkState *state, HkValue *args);
static void verify_signature_call(HkState *state, HkValue *args);

static void new_key_pair_call(HkState *state, HkValue *args)
{
  (void) args;
  HkString *pub_key = hk_string_new_with_capacity(PUBLIC_KEY_SIZE);
  pub_key->length = PUBLIC_KEY_SIZE;
  pub_key->chars[PUBLIC_KEY_SIZE] = '\0';
  HkString *priv_key = hk_string_new_with_capacity(PRIVATE_KEY_SIZE);
  priv_key->length = PRIVATE_KEY_SIZE;
  priv_key->chars[PRIVATE_KEY_SIZE] = '\0';
  (void) ecc_make_key((uint8_t *) pub_key->chars, (uint8_t *) priv_key->chars);
  HkArray *arr = hk_array_new_with_capacity(2);
  arr->length = 2;
  hk_incr_ref(pub_key);
  hk_incr_ref(priv_key);
  arr->elements[0] = hk_string_value(pub_key);
  arr->elements[1] = hk_string_value(priv_key);
  hk_state_push_array(state, arr);
  if (!hk_state_is_ok(state))
    hk_array_free(arr);
}

static void shared_secret_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  HkString *pub_key = hk_as_string(args[1]);
  HkString *priv_key = hk_as_string(args[2]);
  HkString *secret = hk_string_new_with_capacity(SECRET_SIZE);
  secret->length = SECRET_SIZE;
  secret->chars[SECRET_SIZE] = '\0';
  (void) ecdh_shared_secret((uint8_t *) pub_key->chars, (uint8_t *) priv_key->chars,
    (uint8_t *) secret->chars);
  hk_state_push_string(state, secret);
  if (!hk_state_is_ok(state))
    hk_string_free(secret);
}

static void sign_hash_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  HkString *priv_key = hk_as_string(args[1]);
  HkString *hash = hk_as_string(args[2]);
  HkString *signature = hk_string_new_with_capacity(SIGNATURE_SIZE);
  signature->length = SIGNATURE_SIZE;
  signature->chars[SIGNATURE_SIZE] = '\0';
  (void) ecdsa_sign((uint8_t *) priv_key->chars, (uint8_t *) hash->chars,
    (uint8_t *) signature->chars);
  hk_state_push_string(state, signature);
  if (!hk_state_is_ok(state))
    hk_string_free(signature);
}

static void verify_signature_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 3);
  hk_return_if_not_ok(state);
  HkString *pub_key = hk_as_string(args[1]);
  HkString *hash = hk_as_string(args[2]);
  HkString *signature = hk_as_string(args[3]);
  bool valid = (bool) ecdsa_verify((uint8_t *) pub_key->chars, (uint8_t *) hash->chars,
    (uint8_t *) signature->chars);
  hk_state_push_bool(state, valid);
}

HK_LOAD_MODULE_HANDLER(secp256r1)
{
  hk_state_push_string_from_chars(state, -1, "secp256r1");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "PUBLIC_KEY_SIZE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, PUBLIC_KEY_SIZE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "PRIVATE_KEY_SIZE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, PRIVATE_KEY_SIZE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "SECRET_SIZE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, SECRET_SIZE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "HASH_SIZE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, HASH_SIZE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "SIGNATURE_SIZE");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, SIGNATURE_SIZE);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new_key_pair");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new_key_pair", 0, new_key_pair_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "shared_secret");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "shared_secret", 2, shared_secret_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "sign_hash");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "sign_hash", 2, sign_hash_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "verify_signature");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "verify_signature", 3, verify_signature_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 9);
}
