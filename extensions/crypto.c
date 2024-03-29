//
// The Hook Programming Language
// crypto.c
//

#include "crypto.h"
#include <openssl/rand.h>
#include "deps/rc4.h"

static void random_bytes_call(HkState *state, HkValue *args);
static void rc4_encrypt_call(HkState *state, HkValue *args);
static void rc4_decrypt_call(HkState *state, HkValue *args);

static void random_bytes_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_int(state, args, 1);
  hk_return_if_not_ok(state);
  int length = (int) (int) hk_as_number(args[1]);
  HkString *str = hk_string_new_with_capacity(length);
  if (!RAND_bytes((unsigned char *) str->chars, length))
  {
    hk_state_push_nil(state);
    return;
  }
  str->length = length;
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

static void rc4_encrypt_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  HkString *key = hk_as_string(args[1]);
  HkString *input = hk_as_string(args[2]);
  int key_length = key->length;
  HkString *err = NULL;
  HkString *output = NULL;
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
  int length = input->length;
  output = hk_string_new_with_capacity(length);
  rc4_ctx ctx;
  rc4_ks(&ctx, (uint8 *) key->chars, (uint32) key_length);
  rc4_encrypt(&ctx, (uint8 *) input->chars, (uint8 *) output->chars, (uint32) length);
  output->length = length;
  output->chars[length] = '\0';
  HkArray *arr;
end:
  hk_assert(err || output, "err or output must be non-NULL");
  arr = hk_array_new_with_capacity(2);
  hk_array_inplace_add_element(arr, err ? HK_NIL_VALUE : hk_string_value(output));
  hk_array_inplace_add_element(arr, err ? hk_string_value(err) : HK_NIL_VALUE);
  hk_state_push_array(state, arr);
  if (!hk_state_is_ok(state))
    hk_array_free(arr);
}

static void rc4_decrypt_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  HkString *key = hk_as_string(args[1]);
  HkString *input = hk_as_string(args[2]);
  int key_length = key->length;
  HkString *err = NULL;
  HkString *output = NULL;
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
  int length = input->length;
  output = hk_string_new_with_capacity(length);
  rc4_ctx ctx;
  rc4_ks(&ctx, (uint8 *) key->chars, (uint32) key_length);
  rc4_decrypt(&ctx, (uint8 *) input->chars, (uint8 *) output->chars, (uint32) length);
  output->length = length;
  output->chars[length] = '\0';
  HkArray *arr;
end:
  hk_assert(err || output, "err or output must be non-NULL");
  arr = hk_array_new_with_capacity(2);
  hk_array_inplace_add_element(arr, err ? HK_NIL_VALUE : hk_string_value(output));
  hk_array_inplace_add_element(arr, err ? hk_string_value(err) : HK_NIL_VALUE);
  hk_state_push_array(state, arr);
  if (!hk_state_is_ok(state))
    hk_array_free(arr);
}

HK_LOAD_MODULE_HANDLER(crypto)
{
  hk_state_push_string_from_chars(state, -1, "crypto");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "random_bytes");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "random_bytes", 1, random_bytes_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "rc4_encrypt");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "rc4_encrypt", 2, rc4_encrypt_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "rc4_decrypt");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "rc4_decrypt", 2, rc4_decrypt_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 3);
}
