//
// The Hook Programming Language
// module.c
//

#include "module.h"
#include <stdlib.h>
#include <hook/status.h>
#include <hook/error.h>
#include <hook/utils.h>
#include "string_map.h"

#ifdef _WIN32
  #include <Windows.h>
#endif

#ifndef _WIN32
  #include <dlfcn.h>
#endif

#define HOME_ENV_VAR "HOOK_HOME"

#ifdef _WIN32
  #define FILE_INFIX   "\\lib\\"
  #define FILE_POSTFIX "_mod.dll"
#endif

#ifndef _WIN32
  #define FILE_INFIX "/lib/lib"
#endif

#ifdef __linux__
  #define FILE_POSTFIX "_mod.so"
#endif

#ifdef __APPLE__
  #define FILE_POSTFIX "_mod.dylib"
#endif

#ifdef _WIN32
  typedef int (__stdcall *load_module_t)(HkState *);
#else
  typedef int (*load_module_t)(HkState *);
#endif

static StringMap module_cache;

static inline bool get_module_result(HkString *name, HkValue *result);
static inline void put_module_result(HkString *name, HkValue result);
static inline const char *get_home_dir(void);
static inline const char *get_default_home_dir(void);

static inline int load_native_module(HkState *state, HkString *name);

static inline bool get_module_result(HkString *name, HkValue *result)
{
  StringMapEntry *entry = string_map_get_entry(&module_cache, name);
  if (!entry)
    return false;
  *result = entry->value;
  return true;
}

static inline void put_module_result(HkString *name, HkValue result)
{
  string_map_inplace_put(&module_cache, name, result);
}

static inline const char *get_home_dir(void)
{
  const char *result = getenv(HOME_ENV_VAR);
  if (!result)
    result = get_default_home_dir();
  return result;
}

static inline const char *get_default_home_dir(void)
{
  const char *result = "/opt/hook";
#ifdef _WIN32
  const char *drive = getenv("SystemDrive");
  hk_assert(drive, "environment variable 'SystemDrive' not set");
  char *path[MAX_PATH + 1];
  snprintf(path, MAX_PATH, "%s\\hook", drive);
  strncpy_s(path, MAX_PATH, drive, _TRUNCATE);
  result = (const char *) path;
#endif
  return result;
}

static inline int load_native_module(HkState *state, HkString *name)
{
  HkString *file = hk_string_from_chars(-1, get_home_dir());
  hk_string_inplace_concat_chars(file, -1, FILE_INFIX);
  hk_string_inplace_concat(file, name);
  hk_string_inplace_concat_chars(file, -1, FILE_POSTFIX);
#ifdef _WIN32
  HINSTANCE handle = LoadLibrary(file->chars);
#else
  void *handle = dlopen(file->chars, RTLD_NOW | RTLD_GLOBAL);
#endif
  if (!handle)
  {
    hk_runtime_error("cannot open module `%.*s`", file->length, file->chars);
    hk_string_free(file);
    return HK_STATUS_ERROR;
  }
  hk_string_free(file);
  HkString *fn_name = hk_string_from_chars(-1, HK_LOAD_FN_PREFIX);
  hk_string_inplace_concat(fn_name, name);
  load_module_t load;
#ifdef _WIN32
  load = (load_module_t) GetProcAddress(handle, fn_name->chars);
#else
  *((void **) &load) = dlsym(handle, fn_name->chars);
#endif
  if (!load)
  {
    hk_runtime_error("no such function %.*s()", fn_name->length, fn_name->chars);
    hk_string_free(fn_name);
    return HK_STATUS_ERROR;
  }
  hk_string_free(fn_name);
  if (load(state) == HK_STATUS_ERROR)
  {
    hk_runtime_error("cannot load module `%.*s`", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

void init_module_cache(void)
{
  string_map_init(&module_cache, 0);
}

void free_module_cache(void)
{
  string_map_free(&module_cache);
}

int load_module(HkState *state)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  hk_assert(hk_is_string(val), "module name must be a string");
  HkString *name = hk_as_string(val);
  HkValue result;
  if (get_module_result(name, &result))
  {
    hk_value_incr_ref(result);
    slots[0] = result;
    --state->stackTop;
    hk_string_release(name);
    return HK_STATUS_OK;
  }
  if (load_native_module(state, name) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  put_module_result(name, state->stackSlots[state->stackTop]);
  slots[0] = state->stackSlots[state->stackTop];
  --state->stackTop;
  hk_string_release(name);
  return HK_STATUS_OK;
}
