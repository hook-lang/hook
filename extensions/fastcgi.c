//
// The Hook Programming Language
// fastcgi.c
//

#include "fastcgi.h"
#include <fcgi_stdio.h>

static void accept_call(HkState *state, HkValue *args);

static void accept_call(HkState *state, HkValue *args)
{
  (void) args;
  hk_state_push_number(state, FCGI_Accept());
}

HK_LOAD_MODULE_HANDLER(fastcgi)
{
  hk_state_push_string_from_chars(state, -1, "fastcgi");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "accept");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "accept", 0, &accept_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 1);
}
