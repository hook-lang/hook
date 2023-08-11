//
// The Hook Programming Language
// strings.c
//

#include "strings.h"
#include <string.h>

static void new_string_call(HkState *state, HkValue *args);
static void repeat_call(HkState *state, HkValue *args);
static void hash_call(HkState *state, HkValue *args);
static void lower_call(HkState *state, HkValue *args);
static void upper_call(HkState *state, HkValue *args);
static void trim_call(HkState *state, HkValue *args);
static void starts_with_call(HkState *state, HkValue *args);
static void ends_with_call(HkState *state, HkValue *args);
static void reverse_call(HkState *state, HkValue *args);

static void new_string_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_int(state, args, 1);
  hk_return_if_not_ok(state);
  int capacity = (int) hk_as_number(args[1]);
  HkString *str = hk_string_new_with_capacity(capacity);
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

static void repeat_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
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
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void hash_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, hk_string_hash(hk_as_string(args[1])));
}

static void lower_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_string_lower(hk_as_string(args[1]));
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

static void upper_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_string_upper(hk_as_string(args[1]));
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

static void trim_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str;
  if (!hk_string_trim(hk_as_string(args[1]), &str))
    return;
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

static void starts_with_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_push_bool(state, hk_string_starts_with(hk_as_string(args[1]), hk_as_string(args[2])));
}

static void ends_with_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_push_bool(state, hk_string_ends_with(hk_as_string(args[1]), hk_as_string(args[2])));
}

static void reverse_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *str = hk_string_reverse(hk_as_string(args[1]));
  hk_state_push_string(state, str);
  if (!hk_state_is_ok(state))
    hk_string_free(str);
}

HK_LOAD_MODULE_HANDLER(strings)
{
  hk_state_push_string_from_chars(state, -1, "strings");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new_string");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new_string", 1, new_string_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "repeat");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "repeat", 2, repeat_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "hash");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "hash", 1, hash_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "lower");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "lower", 1, lower_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "upper");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "upper", 1, upper_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "trim");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "trim", 1, trim_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "starts_with");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "starts_with", 2, starts_with_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "ends_with");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "ends_with", 2, ends_with_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "reverse");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "reverse", 1, reverse_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 9);
}
