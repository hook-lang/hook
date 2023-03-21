//
// The Hook Programming Language
// crypto.c
//

#include "crypto.h"
#include "deps/rc4.h"
#include <openssl/rand.h>
#include <hook/check.h>
#include <hook/status.h>

static int32_t random_bytes_call(hk_state_t *state, hk_value_t *args);
static int32_t rc4_encrypt_call(hk_state_t *state, hk_value_t *args);
static int32_t rc4_decrypt_call(hk_state_t *state, hk_value_t *args);

static int32_t random_bytes_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_int(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  int32_t length = (int32_t) (int32_t) hk_as_number(args[1]);
  hk_string_t *str = hk_string_new_with_capacity(length);
  if (!RAND_bytes((unsigned char *) str->chars, length))
    return hk_state_push_nil(state);
  str->length = length;
  if (hk_state_push_string(state, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t rc4_encrypt_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *key = hk_as_string(args[1]);
  hk_string_t *input = hk_as_string(args[2]);
  int32_t key_length = key->length;
  hk_string_t *err = NULL;
  if (key_length < 1)
  {
    err = hk_string_from_chars(-1, "key length must be greater than 0");
    goto end;
  }
  if (key_length > 256)
  {
    err = hk_string_from_chars(-1, "key length must be less than or equal to 256");
    goto end;
  }
  int32_t length = input->length;
  hk_string_t *output = hk_string_new_with_capacity(length);
  rc4_ctx ctx;
  rc4_ks(&ctx, (uint8 *) key->chars, (uint32) key_length);
  rc4_encrypt(&ctx, (uint8 *) input->chars, (uint8 *) output->chars, (uint32) length);
  output->length = length;
  output->chars[length] = '\0';
  hk_array_t *arr;
end:
  arr = hk_array_new_with_capacity(2);
  hk_array_inplace_add_element(arr, err ? HK_NIL_VALUE : hk_string_value(output));
  hk_array_inplace_add_element(arr, err ? hk_string_value(err) : HK_NIL_VALUE);
  if (hk_state_push_array(state, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int32_t rc4_decrypt_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *key = hk_as_string(args[1]);
  hk_string_t *input = hk_as_string(args[2]);
  int32_t key_length = key->length;
  hk_string_t *err = NULL;
  if (key_length < 1)
  {
    err = hk_string_from_chars(-1, "key length must be greater than 0");
    goto end;
  }
  if (key_length > 256)
  {
    err = hk_string_from_chars(-1, "key length must be less than or equal to 256");
    goto end;
  }
  int32_t length = input->length;
  hk_string_t *output = hk_string_new_with_capacity(length);
  rc4_ctx ctx;
  rc4_ks(&ctx, (uint8 *) key->chars, (uint32) key_length);
  rc4_decrypt(&ctx, (uint8 *) input->chars, (uint8 *) output->chars, (uint32) length);
  output->length = length;
  output->chars[length] = '\0';
  hk_array_t *arr;
end:
  arr = hk_array_new_with_capacity(2);
  hk_array_inplace_add_element(arr, err ? HK_NIL_VALUE : hk_string_value(output));
  hk_array_inplace_add_element(arr, err ? hk_string_value(err) : HK_NIL_VALUE);
  if (hk_state_push_array(state, arr) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

HK_LOAD_FN(crypto)
{
  if (hk_state_push_string_from_chars(state, -1, "crypto") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "random_bytes") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "random_bytes", 1, &random_bytes_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "rc4_encrypt") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "rc4_encrypt", 2, &rc4_encrypt_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "rc4_decrypt") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "rc4_decrypt", 2, &rc4_decrypt_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;  
  return hk_state_construct(state, 3);
}
