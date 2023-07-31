//
// The Hook Programming Language
// strings.c
//

#include "strings.h"
#include <string.h>
#include <hook/check.h>
#include <hook/status.h>

static int new_string_call(HkState *state, HkValue *args);
static int repeat_call(HkState *state, HkValue *args);
static int hash_call(HkState *state, HkValue *args);
static int lower_call(HkState *state, HkValue *args);
static int upper_call(HkState *state, HkValue *args);
static int trim_call(HkState *state, HkValue *args);
static int starts_with_call(HkState *state, HkValue *args);
static int ends_with_call(HkState *state, HkValue *args);
static int reverse_call(HkState *state, HkValue *args);

static int new_string_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_int(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  int capacity = (int) hk_as_number(args[1]);
  HkString *str = hk_string_new_with_capacity(capacity);
  if (hk_state_push_string(state, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int repeat_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  int count = (int) hk_as_number(args[2]);
  count = count < 0 ? 0 : count;
  int length = str->length;
  int new_length = length * count;
  HkString *result = hk_string_new_with_capacity(new_length);
  char *src = str->chars;
  char *dest = result->chars;
  for (int i = 0; i < count; ++i)
  {
    memcpy(dest, src, length);
    dest += length;
  }
  result->length = new_length;
  result->chars[new_length] = '\0';
  if (hk_state_push_string(state, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int hash_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, hk_string_hash(hk_as_string(args[1])));
}

static int lower_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_string_lower(hk_as_string(args[1]));
  if (hk_state_push_string(state, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int upper_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_string_upper(hk_as_string(args[1]));
  if (hk_state_push_string(state, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int trim_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str;
  if (!HkStringrim(hk_as_string(args[1]), &str))
    return HK_STATUS_OK;
  if (hk_state_push_string(state, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int starts_with_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_bool(state, hk_string_starts_with(hk_as_string(args[1]), hk_as_string(args[2])));
}

static int ends_with_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_bool(state, hk_string_ends_with(hk_as_string(args[1]), hk_as_string(args[2])));
}

static int reverse_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_string_reverse(hk_as_string(args[1]));
  if (hk_state_push_string(state, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

HK_LOAD_FN(strings)
{
  if (hk_state_push_string_from_chars(state, -1, "strings") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "new_string") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "new_string", 1, &new_string_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "repeat") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "repeat", 2, &repeat_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "hash") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "hash", 1, &hash_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "lower") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "lower", 1, &lower_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "upper") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "upper", 1, &upper_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "trim") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "trim", 1, &trim_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "starts_with") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "starts_with", 2, &starts_with_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "ends_with") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "ends_with", 2, &ends_with_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "reverse") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "reverse", 1, &reverse_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 9);
}
