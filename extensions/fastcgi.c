//
// The Hook Programming Language
// fastcgi.c
//

#include "fastcgi.h"
#include <fcgi_stdio.h>
#include "hk_status.h"

static int32_t accept_call(hk_state_t *state, hk_value_t *args);

static int32_t accept_call(hk_state_t *state, hk_value_t *args)
{
  (void) args;
  return hk_state_push_number(state, FCGI_Accept());
}

HK_LOAD_FN(fastcgi)
{
  if (hk_state_push_string_from_chars(state, -1, "fastcgi") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "accept") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "accept", 0, &accept_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 1);
}
