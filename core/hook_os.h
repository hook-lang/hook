//
// Hook Programming Language
// hook_os.h
//

#ifndef HOOK_OS_H
#define HOOK_OS_H

#include "hook.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_os(hk_vm_t *vm);
#else
int32_t load_os(hk_vm_t *vm);
#endif

#endif // HOOK_OS_H
