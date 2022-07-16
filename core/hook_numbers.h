//
// Hook Programming Language
// hook_numbers.h
//

#ifndef HOOK_NUMBERS_H
#define HOOK_NUMBERS_H

#include "hook.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_numbers(hk_vm_t *vm);
#else
int32_t load_numbers(hk_vm_t *vm);
#endif

#endif // HOOK_NUMBERS_H
