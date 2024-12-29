//
// secp256r1.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "secp256r1.h"
#define ECC_CURVE 32
#include "deps/ecc.h"

#define PUBLIC_KEY_SIZE  (ECC_BYTES + 1)
#define PRIVATE_KEY_SIZE ECC_BYTES
#define SECRET_SIZE      ECC_BYTES
#define HASH_SIZE        ECC_BYTES
#define SIGNATURE_SIZE   (ECC_BYTES << 1)

static void new_key_pair_call(HkVM *vm, HkValue *args);
static void shared_secret_call(HkVM *vm, HkValue *args);
static void sign_hash_call(HkVM *vm, HkValue *args);
static void verify_signature_call(HkVM *vm, HkValue *args);

static void new_key_pair_call(HkVM *vm, HkValue *args)
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
  hk_vm_push_array(vm, arr);
  if (!hk_vm_is_ok(vm))
    hk_array_free(arr);
}

static void shared_secret_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkString *pub_key = hk_as_string(args[1]);
  HkString *priv_key = hk_as_string(args[2]);
  HkString *secret = hk_string_new_with_capacity(SECRET_SIZE);
  secret->length = SECRET_SIZE;
  secret->chars[SECRET_SIZE] = '\0';
  (void) ecdh_shared_secret((uint8_t *) pub_key->chars, (uint8_t *) priv_key->chars,
    (uint8_t *) secret->chars);
  hk_vm_push_string(vm, secret);
  if (!hk_vm_is_ok(vm))
    hk_string_free(secret);
}

static void sign_hash_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkString *priv_key = hk_as_string(args[1]);
  HkString *hash = hk_as_string(args[2]);
  HkString *signature = hk_string_new_with_capacity(SIGNATURE_SIZE);
  signature->length = SIGNATURE_SIZE;
  signature->chars[SIGNATURE_SIZE] = '\0';
  (void) ecdsa_sign((uint8_t *) priv_key->chars, (uint8_t *) hash->chars,
    (uint8_t *) signature->chars);
  hk_vm_push_string(vm, signature);
  if (!hk_vm_is_ok(vm))
    hk_string_free(signature);
}

static void verify_signature_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 3);
  hk_return_if_not_ok(vm);
  HkString *pub_key = hk_as_string(args[1]);
  HkString *hash = hk_as_string(args[2]);
  HkString *signature = hk_as_string(args[3]);
  bool valid = (bool) ecdsa_verify((uint8_t *) pub_key->chars, (uint8_t *) hash->chars,
    (uint8_t *) signature->chars);
  hk_vm_push_bool(vm, valid);
}

HK_LOAD_MODULE_HANDLER(secp256r1)
{
  hk_vm_push_string_from_chars(vm, -1, "secp256r1");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "PUBLIC_KEY_SIZE");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, PUBLIC_KEY_SIZE);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "PRIVATE_KEY_SIZE");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, PRIVATE_KEY_SIZE);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SECRET_SIZE");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, SECRET_SIZE);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "HASH_SIZE");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, HASH_SIZE);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "SIGNATURE_SIZE");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, SIGNATURE_SIZE);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "new_key_pair");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "new_key_pair", 0, new_key_pair_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "shared_secret");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "shared_secret", 2, shared_secret_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "sign_hash");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "sign_hash", 2, sign_hash_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "verify_signature");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "verify_signature", 3, verify_signature_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 9);
}
