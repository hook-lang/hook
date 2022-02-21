//
// Hook Programming Language
// h_math.h
//

#ifndef H_MATH_H
#define H_MATH_H

#include "h_vm.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_math(vm_t *vm);
#else
int load_math(vm_t *vm);
#endif

#endif // H_MATH_H
