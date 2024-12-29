//
// ini.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "ini.h"
#include "deps/ini.h"

typedef struct
{
  HK_USERDATA_HEADER
  ini_t *config;
} IniWrapper;

static inline IniWrapper *ini_wrapper_new(ini_t *config);
static void ini_wrapper_deinit(HkUserdata *udata);
static void load_call(HkVM *vm, HkValue *args);
static void get_call(HkVM *vm, HkValue *args);

static inline IniWrapper *ini_wrapper_new(ini_t *config)
{
  IniWrapper *wrapper = (IniWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, ini_wrapper_deinit);
  wrapper->config = config;
  return wrapper;
}

static void ini_wrapper_deinit(HkUserdata *udata)
{
  ini_free(((IniWrapper *) udata)->config);
}

static void load_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *filename = hk_as_string(args[1]);
  ini_t *config = ini_load(filename->chars);
  if (!config)
  {
    hk_vm_push_nil(vm);
    return;
  }
  HkUserdata *udata = (HkUserdata *) ini_wrapper_new(config);
  hk_vm_push_userdata(vm, udata);
  if (!hk_vm_is_ok(vm))
    ini_wrapper_deinit(udata);
}

static void get_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 3);
  hk_return_if_not_ok(vm);
  IniWrapper *wrapper = (IniWrapper *) hk_as_userdata(args[1]);
  HkString *section = hk_as_string(args[2]);
  HkString *key = hk_as_string(args[3]);
  const char *value = ini_get(wrapper->config, section->chars, key->chars);
  if (!value)
  {
    hk_vm_push_nil(vm);
    return;
  }
  hk_vm_push_string_from_chars(vm, -1, value);
}

HK_LOAD_MODULE_HANDLER(ini)
{
  hk_vm_push_string_from_chars(vm, -1, "ini");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "load");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "load", 1, load_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "get");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "get", 3, get_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 2);
}
