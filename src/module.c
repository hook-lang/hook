//
// Hook Programming Language
// module.c
//

#include "module.h"
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif
#include "common.h"
#include "error.h"

#define HOME_VAR "HOOK_HOME"

#ifdef _WIN32
#define FILE_INFIX "\\lib\\"
#define FILE_EXT   ".dll"
#else
#define FILE_INFIX "/lib/lib"
#ifdef __APPLE__
#define FILE_EXT   ".dylib"
#else
#define FILE_EXT   ".so"
#endif
#endif

#define FUNC_PREFIX "load_"

#ifdef _WIN32
typedef int (__stdcall *load_module_t)(vm_t *);
#else
typedef int (*load_module_t)(vm_t *);
#endif

static inline const char *get_home_dir(void);

static inline const char *get_home_dir(void)
{
  const char *home_dir = getenv(HOME_VAR);
  if (!home_dir)
  {
#ifdef _WIN32
    const char *drive = getenv("SystemDrive");
    ASSERT(drive, "environment variable 'SystemDrive' not set");
    char *path[MAX_PATH + 1];
    snprintf(path, MAX_PATH, "%s\\hook", drive);
    strncpy(path, drive, MAX_PATH);
    home_dir = (const char *) path;
#else
    home_dir = "/usr/local/hook";
#endif
  }
  return home_dir;
}

int load_module(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top];
  value_t val = slots[0];
  ASSERT(IS_STRING(val), "module name must be a string");
  string_t *name = AS_STRING(val);
  string_t *file = string_from_chars(-1, get_home_dir());
  string_inplace_concat_chars(file, -1, FILE_INFIX);
  string_inplace_concat(file, name);
  string_inplace_concat_chars(file, -1, FILE_EXT);
#ifdef _WIN32
  HINSTANCE handle = LoadLibrary(file->chars);
#else
  void *handle = dlopen(file->chars, RTLD_NOW | RTLD_GLOBAL);
#endif
  if (!handle)
  {
    runtime_error("cannot open module `%.*s`", name->length, name->chars);
    string_free(file);
    return STATUS_ERROR;
  }
  string_free(file);
  string_t *func = string_from_chars(-1, FUNC_PREFIX);
  string_inplace_concat(func, name);
  DECR_REF(name);
  if (IS_UNREACHABLE(name))
    string_free(name);
#ifdef _WIN32
  load_module_t load = (load_module_t) GetProcAddress(handle, func->chars);
#else
  load_module_t load = dlsym(handle, func->chars);
#endif
  if (!load)
  {
    runtime_error("no such function %.*s()", func->length, func->chars);
    string_free(func);
    return STATUS_ERROR;
  }
  string_free(func);
  --vm->top;
  if (load(vm) == STATUS_ERROR)
  {
    runtime_error("cannot load module `%.*s`", name->length, name->chars);
    return STATUS_ERROR;
  }
  return STATUS_OK;
}
