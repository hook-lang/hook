//
// The Hook Programming Language
// uws.c
//

#include "uws.h"

static void new_app_call(HkState *state, HkValue *args);
static void listen_call(HkState *state, HkValue *args);
static void run_call(HkState *state, HkValue *args);

static void new_app_call(HkState *state, HkValue *args)
{
  // TODO: Implement this function.
  (void) args;
  hk_state_push_nil(state);
}

static void listen_call(HkState *state, HkValue *args)
{
  // TODO: Implement this function.
  (void) args;
  hk_state_push_nil(state);
}

static void run_call(HkState *state, HkValue *args)
{
  // TODO: Implement this function.
  (void) args;
  hk_state_push_nil(state);
}

HK_LOAD_MODULE_HANDLER(uws)
{
  hk_state_push_string_from_chars(state, -1, "uws");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new_app");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new_app", 2, new_app_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "listen");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "listen", 2, listen_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "run");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "run", 1, run_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 3);
}
