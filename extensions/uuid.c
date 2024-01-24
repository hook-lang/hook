//
// The Hook Programming Language
// uuid.c
//

#include "uuid.h"
#include "deps/uuid4.h"

static void random_call(HkState *state, HkValue *args);

static void random_call(HkState *state, HkValue *args)
{
  (void) args;
  HkString *str = hk_string_new_with_capacity(UUID4_LEN);
  uuid4_init();
  uuid4_generate(str->chars);
  str->length = UUID4_LEN;
  str->chars[str->length] = '\0';
  hk_state_push_string(state, str);
}

HK_LOAD_MODULE_HANDLER(uuid)
{
  hk_state_push_string_from_chars(state, -1, "uuid");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "random");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "random", 0, random_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 1);
}
