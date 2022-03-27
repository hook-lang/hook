//
// Hook Programming Language
// hook_strings.h
//

#ifndef HOOK_STRINGS_H
#define HOOK_STRINGS_H

#include "hook.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_strings(hk_vm_t *vm);
#else
int32_t load_strings(hk_vm_t *vm);
#endif

#endif // HOOK_STRINGS_H
