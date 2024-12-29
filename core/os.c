//
// os.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
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

static void clock_call(HkVM *vm, HkValue *args);
static void time_call(HkVM *vm, HkValue *args);
static void system_call(HkVM *vm, HkValue *args);
static void getenv_call(HkVM *vm, HkValue *args);
static void getcwd_call(HkVM *vm, HkValue *args);
static void name_call(HkVM *vm, HkValue *args);

static void clock_call(HkVM *vm, HkValue *args)
{
  (void) args;
  hk_vm_push_number(vm, (double) clock() / CLOCKS_PER_SEC);
}

static void time_call(HkVM *vm, HkValue *args)
{
  (void) args;
  hk_vm_push_number(vm, (double) time(NULL));
}

static void system_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, system(hk_as_string(args[1])->chars));
}

static void getenv_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  const char *chars = getenv(hk_as_string(args[1])->chars);
  chars = chars ? chars : "";
  hk_vm_push_string_from_chars(vm, -1, chars);
}

static void getcwd_call(HkVM *vm, HkValue *args)
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
    hk_vm_push_nil(vm);
    return;
  }
  hk_vm_push_string(vm, result);
  if (!hk_vm_is_ok(vm))
    hk_string_free(result);
}

static void name_call(HkVM *vm, HkValue *args)
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
  hk_vm_push_string_from_chars(vm, -1, result);
}

HK_LOAD_MODULE_HANDLER(os)
{
  hk_vm_push_string_from_chars(vm, -1, "os");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "CLOCKS_PER_SEC");
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, CLOCKS_PER_SEC);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "clock");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "clock", 0, clock_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "time");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "time", 0, time_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "system");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "system", 1, system_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "getenv");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "getenv", 1, getenv_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "getcwd");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "getcwd", 1, getcwd_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "name");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "name", 0, name_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 7);
}
