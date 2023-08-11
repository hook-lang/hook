//
// The Hook Programming Language
// numbers.c
//

#include "numbers.h"
#include <stdlib.h>
#include <float.h>

#define PI  3.14159265358979323846264338327950288
#define TAU 6.28318530717958647692528676655900577

#define LARGEST  DBL_MAX
#define SMALLEST DBL_MIN

#define MAX_INTEGER 9007199254740991.0
#define MIN_INTEGER -9007199254740991.0

static void srand_call(HkState *state, HkValue *args);
static void rand_call(HkState *state, HkValue *args);

static void srand_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_number(state, args, 1);
  hk_return_if_not_ok(state);
  srand((uint32_t) hk_as_number(args[1]));
  hk_state_push_nil(state);
}

static void rand_call(HkState *state, HkValue *args)
{
  (void) args;
  double result = (double) rand() / RAND_MAX;
  hk_state_push_number(state, result);
}

HK_LOAD_MODULE_HANDLER(numbers)
{
  hk_state_push_string_from_chars(state, -1, "numbers");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "PI");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, PI);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "TAU");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, TAU);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "LARGEST");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, LARGEST);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "SMALLEST");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, SMALLEST);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "MAX_INTEGER");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, MAX_INTEGER);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "MIN_INTEGER");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, MIN_INTEGER);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "srand");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "srand", 1, srand_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "rand");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "rand", 0, rand_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 8);
}
