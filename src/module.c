//
// The Hook Programming Language
// module.c
//

#include "module.h"
#include <stdlib.h>
#include <string.h>
#include <hook/compiler.h>
#include <hook/utils.h>
#include "string_map.h"

#ifdef _WIN32
  #include <Windows.h>
  #include <io.h>
#endif

#ifndef _WIN32
  #include <dlfcn.h>
  #include <unistd.h>
#endif

#ifdef _WIN32
  #define file_exists(f) (_access((f), 0) != -1)
#else
  #define file_exists(f) (access((f), F_OK) != -1)
#endif

#define HOME_ENV_VAR "HOOK_HOME"
#define PATH_ENV_VAR "HOOK_PATH"

#ifdef _WIN32
  #define DIR_SEP "\\"
#else
  #define DIR_SEP "/"
#endif

#define PATH_SEP ";"
#define WILDCARD "?"

#define LIB_DIR     "lib"
#define LIB_POSTFIX "_mod"

#define SRC_EXT  ".hk"
#define SRC_MAIN "main"

#ifdef _WIN32
  #define LIB_EXT ".dll"
#elif __linux__
  #define LIB_EXT ".so"
#elif __APPLE__
  #define LIB_EXT ".dylib"
#else
  #error "Unsupported platform"
#endif

#ifdef _WIN32
  typedef void (__stdcall *LoadModuleHandler)(HkState *);
#else
  typedef void (*LoadModuleHandler)(HkState *);
#endif

static StringMap moduleCache;
static HkString *path = NULL;

static inline const char *get_home_dir(void);
static inline const char *get_default_home_dir(void);
static inline HkString *get_path(void);
static inline HkString *get_default_path(void);
static inline HkString *path_match(HkString *path, HkString *name, HkString *currFile);
static inline bool is_relative(char *filename);
static inline HkString *get_module_file(HkString *relFile, HkString *currFile);
static inline void load_module(HkState *state, HkString *name, HkString *currFile);
static inline bool is_source_module(char *filename);
static inline void load_source_module(HkState *state, HkString *file, HkString *name);
static inline void load_native_module(HkState *state, HkString *file, HkString *name);
static inline HkString *load_source_from_file(const char *filename);
static inline bool module_cache_get(HkString *name, HkValue *module);
static inline void module_cache_put(HkString *name, HkValue module);

static inline const char *get_home_dir(void)
{
  const char *dir = getenv(HOME_ENV_VAR);
  if (!dir)
    dir = get_default_home_dir();
  return dir;
}

static inline const char *get_default_home_dir(void)
{
  const char *dir = "/opt/hook";
#ifdef _WIN32
  const char *drive = getenv("SystemDrive");
  hk_assert(drive, "environment variable 'SystemDrive' not set");
  char *path[MAX_PATH + 1];
  snprintf(path, MAX_PATH, "%s\\hook", drive);
  strncpy_s(path, MAX_PATH, drive, _TRUNCATE);
  dir = (const char *) path;
#endif
  return dir;
}

static inline HkString *get_path(void)
{
  if (!path)
  {
    const char *_path = getenv(PATH_ENV_VAR);
    path = _path ? hk_string_from_chars(-1, _path) : get_default_path();
  }
  return path;
}

static inline HkString *get_default_path(void)
{
  HkString *path = hk_string_new();
  hk_string_inplace_concat_chars(path, -1, WILDCARD);
  hk_string_inplace_concat_chars(path, -1, PATH_SEP);

  hk_string_inplace_concat_chars(path, -1, get_home_dir());
  hk_string_inplace_concat_chars(path, -1, DIR_SEP);
  hk_string_inplace_concat_chars(path, -1, LIB_DIR);
  hk_string_inplace_concat_chars(path, -1, DIR_SEP);
  hk_string_inplace_concat_chars(path, -1, WILDCARD);
  hk_string_inplace_concat_chars(path, -1, LIB_POSTFIX);
  hk_string_inplace_concat_chars(path, -1, LIB_EXT);
  hk_string_inplace_concat_chars(path, -1, PATH_SEP);

  hk_string_inplace_concat_chars(path, -1, WILDCARD);
  hk_string_inplace_concat_chars(path, -1, SRC_EXT);
  hk_string_inplace_concat_chars(path, -1, PATH_SEP);

  hk_string_inplace_concat_chars(path, -1, WILDCARD);
  hk_string_inplace_concat_chars(path, -1, DIR_SEP);
  hk_string_inplace_concat_chars(path, -1, SRC_MAIN);
  hk_string_inplace_concat_chars(path, -1, SRC_EXT);
  return path;
}

static inline HkString *path_match(HkString *path, HkString *name, HkString *currFile)
{
  HkString *sep = hk_string_from_chars(-1, PATH_SEP);
  HkArray *patterns = hk_string_split(path, sep);
  hk_string_free(sep);
  HkString *wc = hk_string_from_chars(-1, WILDCARD);
  int n = patterns->length;
  for (int i = 0; i < n; ++i)
  {
    HkValue elem = hk_array_get_element(patterns, i);
    HkString *pattern = hk_as_string(elem);
    HkString *file = hk_string_replace_all(pattern, wc, name);
    if (is_relative(file->chars))
    {
      HkString *_file = get_module_file(file, currFile);
      hk_string_free(file);
      file = _file;
    }
    if (file_exists(file->chars))
    {
      hk_string_free(wc);
      hk_array_free(patterns);
      return file;
    }
    hk_string_free(file);
  }
  hk_string_free(wc);
  hk_array_free(patterns);
  return NULL;
}

static inline bool is_relative(char *filename)
{
#ifdef _WIN32
  return filename[0] != DIR_SEP[0] && filename[1] != ':';
#else
  return filename[0] != DIR_SEP[0];
#endif
}

static inline HkString *get_module_file(HkString *relFile, HkString *currFile)
{
  char *chars = currFile->chars;
  char *end = strrchr(chars, DIR_SEP[0]);
  if (end)
  {
    int length = (int) (end - chars);
    HkString *file = hk_string_from_chars(length, chars);
    hk_string_inplace_concat_chars(file, -1, DIR_SEP);
    hk_string_inplace_concat(file, relFile);
    return file;
  }
  return hk_string_copy(relFile);
}

static inline void load_module(HkState *state, HkString *name, HkString *currFile)
{
  HkString *path = get_path();
  HkString *file = path_match(path, name, currFile);
  if (!file)
  {
    hk_state_runtime_error(state, "cannot find module `%.*s`",
      name->length, name->chars);
    return;
  }
  if (is_source_module(file->chars))
  {
    load_source_module(state, file, name);
    return;
  }
  load_native_module(state, file, name);
  hk_string_free(file);
}

static inline bool is_source_module(char *filename)
{
  char *ext = strrchr(filename, '.');
  if (!ext)
    return false;
  return !strcmp(ext, SRC_EXT);
}

static inline void load_source_module(HkState *state, HkString *file, HkString *name)
{
  HkString *source = load_source_from_file(file->chars);
  if (!source)
    hk_state_runtime_error(state, "cannot open module `%.*s`",
      name->length, name->chars);
  HkClosure *cl = hk_compile(file, source);
  hk_state_push_closure(state, cl);
  hk_state_push_array(state, hk_array_new());
  hk_state_call(state, 1);
  if (!hk_state_is_ok(state))
    hk_state_runtime_error(state, "cannot load module `%.*s`",
      name->length, name->chars);
}

static inline void load_native_module(HkState *state, HkString *file, HkString *name)
{
#ifdef _WIN32
  HINSTANCE handle = LoadLibrary(file->chars);
#else
  void *handle = dlopen(file->chars, RTLD_NOW | RTLD_GLOBAL);
#endif
  if (!handle)
  {
    hk_state_runtime_error(state, "cannot open module `%.*s`",
      name->length, name->chars);
    return;
  }
  HkString *funcName = hk_string_from_chars(-1, HK_LOAD_MODULE_HANDLER_PREFIX);
  hk_string_inplace_concat(funcName, name);
  LoadModuleHandler load;
#ifdef _WIN32
  load = (LoadModuleHandler) GetProcAddress(handle, funcName->chars);
#else
  *((void **) &load) = dlsym(handle, funcName->chars);
#endif
  if (!load)
  {
    hk_state_runtime_error(state, "no such function %.*s()",
      funcName->length, funcName->chars);
    hk_string_free(funcName);
    return;
  }
  hk_string_free(funcName);
  load(state);
  if (!hk_state_is_ok(state))
    hk_state_runtime_error(state, "cannot load module `%.*s`",
      name->length, name->chars);
}

static inline HkString *load_source_from_file(const char *filename)
{
  FILE *stream = fopen(filename, "r");
  if (!stream)
    return NULL;
  HkString *source = hk_string_from_stream(stream, '\0');
  (void) fclose(stream);
  return source;
}

static inline bool module_cache_get(HkString *name, HkValue *module)
{
  StringMapEntry *entry = string_map_get_entry(&moduleCache, name);
  if (!entry)
    return false;
  *module = entry->value;
  return true;
}

static inline void module_cache_put(HkString *name, HkValue module)
{
  string_map_inplace_put(&moduleCache, name, module);
}

void module_cache_init(void)
{
  string_map_init(&moduleCache, 0);
}

void module_cache_deinit(void)
{
  string_map_deinit(&moduleCache);
  if (path)
    hk_string_free(path);
}

void module_load(HkState *state, HkString *currFile)
{
  HkValue *slots = &state->stackSlots[state->stackTop];
  HkValue val = slots[0];
  hk_assert(hk_is_string(val), "module name must be a string");
  HkString *name = hk_as_string(val);
  HkValue module;
  // FIXME: Do cache using absolute file path instead of module name
  if (module_cache_get(name, &module))
  {
    hk_value_incr_ref(module);
    slots[0] = module;
    hk_string_release(name);
    return;
  }
  load_module(state, name, currFile);
  hk_return_if_not_ok(state);
  module_cache_put(name, state->stackSlots[state->stackTop]);
  slots[0] = state->stackSlots[state->stackTop];
  --state->stackTop;
  hk_string_release(name);
}
