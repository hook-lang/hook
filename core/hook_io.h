//
// Hook Programming Language
// hook_io.h
//

#ifndef HOOK_IO_H
#define HOOK_IO_H

#include "hook.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_io(hk_vm_t *vm);
#else
int32_t load_io(hk_vm_t *vm);
#endif

#endif // HOOK_IO_H
