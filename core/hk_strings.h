//
// The Hook Programming Language
// hk_strings.h
//

#ifndef HK_STRINGS_H
#define HK_STRINGS_H

#include "hk_vm.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_strings(hk_vm_t *vm);
#else
int32_t load_strings(hk_vm_t *vm);
#endif

#endif // HK_STRINGS_H
