//
// Hook Programming Language
// library.c
//

#include "library.h"
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif
#include "common.h"
#include "error.h"

#define HOME "HOOK_HOME"

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
typedef int (__stdcall *load_library_t)(vm_t *);
#else
typedef void (*load_library_t)(vm_t *);
#endif

int import_library(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top];
  value_t val = slots[0];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  string_t *name = AS_STRING(val);
  char *home = getenv(HOME);
  if (!home)
  {
    runtime_error("environment variable `%s` not defined", HOME);
    return STATUS_ERROR;
  }
  string_t *file = string_from_chars(-1, home);
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
    runtime_error("cannot load library `%.*s`", name->length, name->chars);
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
  load_library_t load = GetProcAddress(handle, func->chars);
#else
  load_library_t load = dlsym(handle, func->chars);
#endif
  if (!load)
  {
    runtime_error("no such function %.*s()", func->length, func->chars);
    string_free(func);
    return STATUS_ERROR;
  }
  string_free(func);
  --vm->top;
  load(vm);
  return STATUS_OK;
}