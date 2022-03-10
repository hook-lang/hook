//
// Hook Programming Language
// secp256r1.c
//

#include "secp256r1.h"
#include "ecc.h"

static int new_key_pair_call(vm_t *vm, value_t *args);
static int sign_hash_call(vm_t *vm, value_t *args);
static int verify_signature_call(vm_t *vm, value_t *args);

static int new_key_pair_call(vm_t *vm, value_t *args)
{
  (void) args;
  int pub_key_len = ECC_BYTES + 1;
  string_t *pub_key = string_allocate(pub_key_len);
  pub_key->length = pub_key_len;
  pub_key->chars[pub_key_len] = '\0';
  int priv_key_len = ECC_BYTES;
  string_t *priv_key = string_allocate(priv_key_len);
  priv_key->length = priv_key_len;
  priv_key->chars[priv_key_len] = '\0';
  (void) ecc_make_key((uint8_t *) pub_key->chars, (uint8_t *) priv_key->chars);
  array_t *arr = array_allocate(2);
  arr->length = 2;
  INCR_REF(pub_key);
  INCR_REF(priv_key);
  arr->elements[0] = STRING_VALUE(pub_key);
  arr->elements[1] = STRING_VALUE(priv_key);
  if (vm_push_array(vm, arr) == STATUS_ERROR)
  {
    array_free(arr);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int sign_hash_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_string(args, 2) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *priv_key = AS_STRING(args[1]);
  string_t *hash = AS_STRING(args[2]);
  int length = ECC_BYTES << 1;
  string_t *signature = string_allocate(length);
  signature->length = length;
  signature->chars[length] = '\0';
  (void) ecdsa_sign((uint8_t *) priv_key->chars, (uint8_t *) hash->chars,
    (uint8_t *) signature->chars);
  if (vm_push_string(vm, signature) == STATUS_ERROR)
  {
    string_free(signature);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}

static int verify_signature_call(vm_t *vm, value_t *args)
{
  if (vm_check_string(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_string(args, 2) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_string(args, 3) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *pub_key = AS_STRING(args[1]);
  string_t *hash = AS_STRING(args[2]);
  string_t *signature = AS_STRING(args[3]);
  bool valid = (bool) ecdsa_verify((uint8_t *) pub_key->chars, (uint8_t *) hash->chars,
    (uint8_t *) signature->chars);
  return vm_push_boolean(vm, valid);
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_secp256r1(vm_t *vm)
#else
int load_secp256r1(vm_t *vm)
#endif
{
  if (vm_push_string_from_chars(vm, -1, "secp256r1") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "new_key_pair") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "new_key_pair", 0, &new_key_pair_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "sign_hash") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "sign_hash", 2, &sign_hash_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "verify_signature") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "verify_signature", 3, &verify_signature_call) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_construct(vm, 3);
}
