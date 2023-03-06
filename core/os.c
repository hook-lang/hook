//
// The Hook Programming Language
// os.c
//

#include "os.h"
#include <stdlib.h>
#include <time.h>
#include <hook/check.h>
#include <hook/status.h>

static int32_t clock_call(hk_state_t *state, hk_value_t *args);
static int32_t time_call(hk_state_t *state, hk_value_t *args);
static int32_t system_call(hk_state_t *state, hk_value_t *args);
static int32_t getenv_call(hk_state_t *state, hk_value_t *args);
static int32_t name_call(hk_state_t *state, hk_value_t *args);

static int32_t clock_call(hk_state_t *state, hk_value_t *args)
{
  (void) args;
  return hk_state_push_number(state, (double) clock() / CLOCKS_PER_SEC);
}

static int32_t time_call(hk_state_t *state, hk_value_t *args)
{
  (void) args;
  return hk_state_push_number(state, (double) time(NULL));
}

static int32_t system_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_push_number(state, system(hk_as_string(args[1])->chars));
}

static int32_t getenv_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  const char *chars = getenv(hk_as_string(args[1])->chars);
  chars = chars ? chars : "";
  return hk_state_push_string_from_chars(state, -1, chars);
}

static int32_t name_call(hk_state_t *state, hk_value_t *args)
{
  (void) args;
  char *result;
#ifdef _WIN32
  result = "windows";
#elif __APPLE__
  result = "macos";
#elif __linux__
  result = "linux";
#elif __unix__
  result = "unix";
#elif __posix__
  result = "posix";
#else
  result = "unknown";
#endif
  return hk_state_push_string_from_chars(state, -1, result);
}

HK_LOAD_FN(os)
{
  if (hk_state_push_string_from_chars(state, -1, "os") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "CLOCKS_PER_SEC") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, CLOCKS_PER_SEC) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "clock") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "clock", 0, &clock_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "time") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "time", 0, &time_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "system") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "system", 1, &system_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "getenv") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "getenv", 1, &getenv_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "name") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "name", 0, &name_call) == HK_STATUS_ERROR) 
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 6);
}
