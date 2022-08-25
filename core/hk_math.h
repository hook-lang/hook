//
// The Hook Programming Language
// hk_math.h
//

#ifndef HK_MATH_H
#define HK_MATH_H

#include "hk_vm.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_math(hk_vm_t *vm);
#else
int32_t load_math(hk_vm_t *vm);
#endif

#endif // HK_MATH_H
