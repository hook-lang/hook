//
// Hook Programming Language
// secp256r1.c
//

#include "secp256r1.h"
#include "ecc.h"

static int new_key_pair_call(hk_vm_t *vm, hk_value_t *args);
static int sign_hash_call(hk_vm_t *vm, hk_value_t *args);
static int verify_signature_call(hk_vm_t *vm, hk_value_t *args);

static int new_key_pair_call(hk_vm_t *vm, hk_value_t *args)
{
  (void) args;
  int pub_key_len = ECC_BYTES + 1;
  hk_string_t *pub_key = hk_string_new_with_capacity(pub_key_len);
  pub_key->length = pub_key_len;
  pub_key->chars[pub_key_len] = '\0';
  int priv_key_len = ECC_BYTES;
  hk_string_t *priv_key = hk_string_new_with_capacity(priv_key_len);
  priv_key->length = priv_key_len;
  priv_key->chars[priv_key_len] = '\0';
  (void) ecc_make_key((uint8_t *) pub_key->chars, (uint8_t *) priv_key->chars);
  hk_array_t *arr = hk_array_new_with_capacity(2);
  arr->length = 2;
  hk_incr_ref(pub_key);
  hk_incr_ref(priv_key);
  arr->elements[0] = hk_string_value(pub_key);
  arr->elements[1] = hk_string_value(priv_key);
  if (hk_vm_push_array(vm, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int sign_hash_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *priv_key = hk_as_string(args[1]);
  hk_string_t *hash = hk_as_string(args[2]);
  int length = ECC_BYTES << 1;
  hk_string_t *signature = hk_string_new_with_capacity(length);
  signature->length = length;
  signature->chars[length] = '\0';
  (void) ecdsa_sign((uint8_t *) priv_key->chars, (uint8_t *) hash->chars,
    (uint8_t *) signature->chars);
  if (hk_vm_push_string(vm, signature) == HK_STATUS_ERROR)
  {
    hk_string_free(signature);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int verify_signature_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *pub_key = hk_as_string(args[1]);
  hk_string_t *hash = hk_as_string(args[2]);
  hk_string_t *signature = hk_as_string(args[3]);
  bool valid = (bool) ecdsa_verify((uint8_t *) pub_key->chars, (uint8_t *) hash->chars,
    (uint8_t *) signature->chars);
  return hk_vm_push_boolean(vm, valid);
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_secp256r1(hk_vm_t *vm)
#else
int load_secp256r1(hk_vm_t *vm)
#endif
{
  if (hk_vm_push_string_from_chars(vm, -1, "secp256r1") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "new_key_pair") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "new_key_pair", 0, &new_key_pair_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "sign_hash") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "sign_hash", 2, &sign_hash_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "verify_signature") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "verify_signature", 3, &verify_signature_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 3);
}
