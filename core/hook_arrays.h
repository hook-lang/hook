//
// Hook Programming Language
// hook_arrays.h
//

#ifndef HOOK_ARRAYS_H
#define HOOK_ARRAYS_H

#include "hook.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_arrays(vm_t *vm);
#else
int load_arrays(vm_t *vm);
#endif

#endif // HOOK_ARRAYS_H
