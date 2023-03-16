//
// The Hook Programming Language
// random.c
//

#include "random.h"
#include <openssl/rand.h>
#include <hook/check.h>
#include <hook/status.h>

static int32_t random_bytes_call(hk_state_t *state, hk_value_t *args);

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

HK_LOAD_FN(random)
{
  if (hk_state_push_string_from_chars(state, -1, "random") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "random_bytes") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "random_bytes", 1, &random_bytes_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 1);
}
