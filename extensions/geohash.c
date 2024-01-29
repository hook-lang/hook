//
// The Hook Programming Language
// geohash.c
//

#include "geohash.h"
#include "deps/geohash.h"

static void encode_call(HkState *state, HkValue *args);
static void decode_call(HkState *state, HkValue *args);

static void encode_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_number(state, args, 2);
  hk_return_if_not_ok(state);
  double lat = hk_as_number(args[1]);
  double lon = hk_as_number(args[2]);
  char buf[16] = {0};
  if (geohash_encode(lat, lon, buf, sizeof(buf)) != GEOHASH_OK)
  {
    hk_state_push_nil(state);
    return;
  }
  HkString *str = hk_string_from_chars(-1, buf);
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

static void decode_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_as_string(args[1]);
  double lat = 0.0;
  double lon = 0.0;
  if (geohash_decode(str->chars, &lat, &lon) != GEOHASH_OK)
  {
    hk_state_push_nil(state);
    return;
  }
  HkArray *arr = hk_array_new_with_capacity(2);
  hk_array_inplace_add_element(arr, hk_number_value(lat));
  hk_array_inplace_add_element(arr, hk_number_value(lon));
  hk_state_push_array(state, arr);
  if (!hk_state_is_ok(state))
    hk_array_free(arr);
}

HK_LOAD_MODULE_HANDLER(geohash)
{
  hk_state_push_string_from_chars(state, -1, "geohash");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "encode");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "encode", 2, encode_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "decode");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "decode", 1, decode_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 2);
}
