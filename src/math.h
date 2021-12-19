//
// Hook Programming Language
// math.h
//

#ifndef MATH_H
#define MATH_H

#include "vm.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_math(vm_t *vm);
#else
int load_math(vm_t *vm);
#endif

#endif // MATH_H
