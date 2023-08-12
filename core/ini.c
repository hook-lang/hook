//
// The Hook Programming Language
// ini.c
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
static void load_call(HkState *state, HkValue *args);
static void get_call(HkState *state, HkValue *args);

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

static void load_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *filename = hk_as_string(args[1]);
  ini_t *config = ini_load(filename->chars);
  if (!config)
  {
    hk_state_push_nil(state);
    return;
  }
  HkUserdata *udata = (HkUserdata *) ini_wrapper_new(config);
  hk_state_push_userdata(state, udata);
  if (!hk_state_is_ok(state))
    ini_wrapper_deinit(udata);
}

static void get_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 3);
  hk_return_if_not_ok(state);
  IniWrapper *wrapper = (IniWrapper *) hk_as_userdata(args[1]);
  HkString *section = hk_as_string(args[2]);
  HkString *key = hk_as_string(args[3]);
  const char *value = ini_get(wrapper->config, section->chars, key->chars);
  if (!value)
  {
    hk_state_push_nil(state);
    return;
  }
  hk_state_push_string_from_chars(state, -1, value);
}

HK_LOAD_MODULE_HANDLER(ini)
{
  hk_state_push_string_from_chars(state, -1, "ini");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "load");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "load", 1, load_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "get");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "get", 3, get_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 2);
}
