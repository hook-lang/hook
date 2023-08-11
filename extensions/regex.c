//
// The Hook Programming Language
// regex.c
//

#include "regex.h"
#include <pcre.h>
#include <hook/memory.h>

typedef struct
{
  HK_USERDATA_HEADER
  pcre *pcre;
} Regex;

static inline Regex *regex_new(pcre *pcre);
static void regex_deinit(HkUserdata *udata);
static void new_call(HkState *state, HkValue *args);
static void is_match_call(HkState *state, HkValue *args);

static inline Regex *regex_new(pcre *pcre)
{
  Regex *regex = (Regex *) hk_allocate(sizeof(*regex));
  hk_userdata_init((HkUserdata *) regex, regex_deinit);
  regex->pcre = pcre;
  return regex;
}

static void regex_deinit(HkUserdata *udata)
{
  pcre_free(((Regex *) udata)->pcre);
}

static void new_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *pattern = hk_as_string(args[1]);
  const char *error;
  int offset;
  pcre *pcre = pcre_compile(pattern->chars, 0, &error, &offset, NULL);
  HkArray *arr = hk_array_new_with_capacity(2);
  if (!pcre)
  {
    char message[128];
    snprintf(message, sizeof(message), "compilation failed at offset %d: %s", offset, error);
    hk_array_inplace_add_element(arr, HK_NIL_VALUE);
    hk_array_inplace_add_element(arr, hk_string_value(hk_string_from_chars(-1, message)));
    hk_state_push_array(state, arr);
    return;
  }
  HkUserdata *udata = (HkUserdata *) regex_new(pcre);
  hk_array_inplace_add_element(arr, hk_userdata_value(udata));
  hk_array_inplace_add_element(arr, HK_NIL_VALUE);
  hk_state_push_array(state, arr);
  if (!hk_state_is_ok(state))
    regex_deinit(udata);
}

static void is_match_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  pcre *pcre = ((Regex *) hk_as_userdata(args[1]))->pcre;
  HkString *subject = hk_as_string(args[2]);
  int rc = pcre_exec(pcre, NULL, subject->chars, subject->length, 0, 0, NULL, 0);
  hk_state_push_bool(state, rc >= 0);
}

HK_LOAD_MODULE_HANDLER(regex)
{
  hk_state_push_string_from_chars(state, -1, "regex");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new", 1, new_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "is_match");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "is_match", 2, is_match_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 2);
}
