//
// The Hook Programming Language
// hk_numbers.h
//

#ifndef HK_NUMBERS_H
#define HK_NUMBERS_H

#include "hk_vm.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_numbers(hk_vm_t *vm);
#else
int32_t load_numbers(hk_vm_t *vm);
#endif

#endif // HK_NUMBERS_H
