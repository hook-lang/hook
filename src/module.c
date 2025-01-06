//
// module.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "module.h"
#include <stdlib.h>
#include <string.h>
#include "hook/compiler.h"
#include "hook/utils.h"
#include "record.h"

#ifdef _WIN32
  #include <io.h>
  #include <Windows.h>
#endif

#ifndef _WIN32
  #include <dlfcn.h>
  #include <limits.h>
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
  #define PATH_MAX MAX_PATH
#endif

#ifdef _WIN32
  typedef void (__stdcall *LoadModuleHandler)(HkVM *);
#else
  typedef void (*LoadModuleHandler)(HkVM *);
#endif

static Record moduleCache;
static HkString *envPath = NULL;

static inline void get_home_dir(char *path);
static inline void get_default_home_dir(char *path);
static inline HkString *get_env_path(void);
static inline HkString *get_default_env_path(void);
static inline HkString *path_match(HkString *path, HkString *name, HkString *currFile);
static inline bool is_relative(char *filename);
static inline HkString *get_module_file(HkString *relFile, HkString *currFile);
static inline void load_module(HkVM *vm, HkString *name, HkString *currFile);
static inline bool is_source_module(char *filename);
static inline void load_source_module(HkVM *vm, HkString *file, HkString *name);
static inline void load_native_module(HkVM *vm, HkString *file, HkString *name);
static inline HkString *load_source_from_file(const char *filename);
static inline bool module_cache_get(HkString *name, HkValue *module);
static inline void module_cache_put(HkString *name, HkValue module);

static inline void get_home_dir(char *path)
{
  const char *dir = getenv(HOME_ENV_VAR);
  if (!dir)
  {
    get_default_home_dir(path);
    return;
  }
  strncpy(path, dir, PATH_MAX);
  path[PATH_MAX] = '\0';
}

static inline void get_default_home_dir(char *path)
{
#ifdef _WIN32
  const char *drive = getenv("SystemDrive");
  hk_assert(drive, "environment variable 'SystemDrive' not set");
  snprintf(path, PATH_MAX + 1, "%s\\hook", drive);
#else
  strncpy(path, "/opt/hook", PATH_MAX);
  path[PATH_MAX] = '\0';
#endif
}

static inline HkString *get_env_path(void)
{
  if (!envPath)
  {
    const char *path = getenv(PATH_ENV_VAR);
    envPath = path ? hk_string_from_chars(-1, path) : get_default_env_path();
  }
  return envPath;
}

static inline HkString *get_default_env_path(void)
{
  char homeDir[PATH_MAX + 1];
  get_home_dir(homeDir);
  HkString *path = hk_string_new();
  hk_string_inplace_concat_chars(path, -1, WILDCARD);
  hk_string_inplace_concat_chars(path, -1, PATH_SEP);
  hk_string_inplace_concat_chars(path, -1, homeDir);
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

static inline void load_module(HkVM *vm, HkString *name, HkString *currFile)
{
  HkString *path = get_env_path();
  HkString *file = path_match(path, name, currFile);
  if (!file)
  {
    hk_vm_runtime_error(vm, "cannot find module `%.*s`",
      name->length, name->chars);
    return;
  }
  if (is_source_module(file->chars))
  {
    load_source_module(vm, file, name);
    return;
  }
  load_native_module(vm, file, name);
  hk_string_free(file);
}

static inline bool is_source_module(char *filename)
{
  char *ext = strrchr(filename, '.');
  if (!ext)
    return false;
  return !strcmp(ext, SRC_EXT);
}

static inline void load_source_module(HkVM *vm, HkString *file, HkString *name)
{
  HkString *source = load_source_from_file(file->chars);
  if (!source)
    hk_vm_runtime_error(vm, "cannot open module `%.*s`",
      name->length, name->chars);
  HkClosure *cl = hk_compile(file, source, HK_COMPILER_FLAG_NONE);
  hk_vm_push_closure(vm, cl);
  hk_vm_push_array(vm, hk_array_new());
  hk_vm_call(vm, 1);
  if (!hk_vm_is_ok(vm))
    hk_vm_runtime_error(vm, "cannot load module `%.*s`",
      name->length, name->chars);
}

static inline void load_native_module(HkVM *vm, HkString *file, HkString *name)
{
#ifdef _WIN32
  HINSTANCE handle = LoadLibrary(file->chars);
#else
  void *handle = dlopen(file->chars, RTLD_NOW | RTLD_GLOBAL);
#endif
  if (!handle)
  {
    hk_vm_runtime_error(vm, "cannot open module `%.*s`",
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
    hk_vm_runtime_error(vm, "no such function %.*s()",
      funcName->length, funcName->chars);
    hk_string_free(funcName);
    return;
  }
  hk_string_free(funcName);
  load(vm);
  if (!hk_vm_is_ok(vm))
    hk_vm_runtime_error(vm, "cannot load module `%.*s`",
      name->length, name->chars);
}

static inline HkString *load_source_from_file(const char *filename)
{
  FILE *stream = NULL;
#ifdef _WIN32
  (void) fopen_s(&stream, filename, "r");
#else
  stream = fopen(filename, "r");
#endif
  if (!stream)
    return NULL;
  HkString *source = hk_string_from_stream(stream, '\0');
  (void) fclose(stream);
  return source;
}

static inline bool module_cache_get(HkString *name, HkValue *module)
{
  RecordEntry *entry = record_get_entry(&moduleCache, name);
  if (!entry)
    return false;
  *module = entry->value;
  return true;
}

static inline void module_cache_put(HkString *name, HkValue module)
{
  record_inplace_put(&moduleCache, name, module);
}

void module_cache_init(void)
{
  record_init(&moduleCache, 0);
}

void module_cache_deinit(void)
{
  record_deinit(&moduleCache);
  if (envPath)
    hk_string_free(envPath);
}

void module_load(HkVM *vm, HkString *currFile)
{
  HkValue *slots = &hk_stack_get(&vm->vstk, 0);
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
  load_module(vm, name, currFile);
  hk_return_if_not_ok(vm);
  val = hk_stack_get(&vm->vstk, 0);
  module_cache_put(name, val);
  slots[0] = val;
  hk_stack_pop(&vm->vstk);
  hk_string_release(name);
}
