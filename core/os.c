//
// The Hook Programming Language
// os.c
//

#include "os.h"
#include <stdlib.h>
#include <time.h>
#include <hook/check.h>
#include <hook/status.h>

#ifdef _WIN32
  #include <windows.h>
#endif

#ifndef _WIN32
  #include <unistd.h>
  #include <limits.h>
#endif

static int32_t clock_call(hk_state_t *state, hk_value_t *args);
static int32_t time_call(hk_state_t *state, hk_value_t *args);
static int32_t system_call(hk_state_t *state, hk_value_t *args);
static int32_t getenv_call(hk_state_t *state, hk_value_t *args);
static int32_t getcwd_call(hk_state_t *state, hk_value_t *args);
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

static int32_t getcwd_call(hk_state_t *state, hk_value_t *args)
{
  (void) args;
  hk_string_t *result = NULL;
#ifdef _WIN32
  TCHAR path[MAX_PATH];
  DWORD length = GetCurrentDirectory(MAX_PATH, path);
  if (!length)
    goto end;
  result = hk_string_from_chars(length, path);
#else
  char path[PATH_MAX];
  if (!getcwd(path, PATH_MAX))
    goto end;
  result = hk_string_from_chars(-1, path);
#endif
end:
  if (!result)
    return hk_state_push_nil(state);
  if (hk_state_push_string(state, result) == HK_STATUS_ERROR)
  {
    hk_string_free(result);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
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
  if (hk_state_push_string_from_chars(state, -1, "getcwd") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "getcwd", 1, &getcwd_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "name") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "name", 0, &name_call) == HK_STATUS_ERROR) 
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 7);
}
