//
// The Hook Programming Language
// hk_numbers.c
//

#include "hk_numbers.h"
#include <float.h>
#include "hk_status.h"

#define PI  3.14159265358979323846264338327950288
#define TAU 6.28318530717958647692528676655900577

#define LARGEST  DBL_MAX
#define SMALLEST DBL_MIN

#define MAX_INTEGER 9007199254740991.0
#define MIN_INTEGER -9007199254740991.0

HK_LOAD_FN(numbers)
{
  if (hk_state_push_string_from_chars(state, -1, "numbers") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "PI") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, PI) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "TAU") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, TAU) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "LARGEST") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, LARGEST) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "SMALLEST") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, SMALLEST) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "MAX_INTEGER") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, MAX_INTEGER) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "MIN_INTEGER") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, MIN_INTEGER) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 6);
}
