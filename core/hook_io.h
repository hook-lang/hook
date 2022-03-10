//
// Hook Programming Language
// hook_io.h
//

#ifndef HOOK_IO_H
#define HOOK_IO_H

#include "hook.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_io(vm_t *vm);
#else
int load_io(vm_t *vm);
#endif

#endif // HOOK_IO_H
