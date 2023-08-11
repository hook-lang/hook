//
// The Hook Programming Language
// os.c
//

#include "os.h"
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
#endif

#ifndef _WIN32
  #include <unistd.h>
  #include <limits.h>
#endif

static void clock_call(HkState *state, HkValue *args);
static void time_call(HkState *state, HkValue *args);
static void system_call(HkState *state, HkValue *args);
static void getenv_call(HkState *state, HkValue *args);
static void getcwd_call(HkState *state, HkValue *args);
static void name_call(HkState *state, HkValue *args);

static void clock_call(HkState *state, HkValue *args)
{
  (void) args;
  hk_state_push_number(state, (double) clock() / CLOCKS_PER_SEC);
}

static void time_call(HkState *state, HkValue *args)
{
  (void) args;
  hk_state_push_number(state, (double) time(NULL));
}

static void system_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, system(hk_as_string(args[1])->chars));
}

static void getenv_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  const char *chars = getenv(hk_as_string(args[1])->chars);
  chars = chars ? chars : "";
  hk_state_push_string_from_chars(state, -1, chars);
}

static void getcwd_call(HkState *state, HkValue *args)
{
  (void) args;
  HkString *result = NULL;
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
  {
    hk_state_push_nil(state);
    return;
  }
  hk_state_push_string(state, result);
  if (!hk_state_is_ok(state))
    hk_string_free(result);
}

static void name_call(HkState *state, HkValue *args)
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
  hk_state_push_string_from_chars(state, -1, result);
}

HK_LOAD_MODULE_HANDLER(os)
{
  hk_state_push_string_from_chars(state, -1, "os");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "CLOCKS_PER_SEC");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, CLOCKS_PER_SEC);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "clock");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "clock", 0, clock_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "time");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "time", 0, time_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "system");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "system", 1, system_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "getenv");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "getenv", 1, getenv_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "getcwd");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "getcwd", 1, getcwd_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "name");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "name", 0, name_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 7);
}
