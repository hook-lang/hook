//
// Hook Programming Language
// math.h
//

#ifndef MATH_H
#define MATH_H

#include "vm.h"

#ifdef WIN32
void __declspec(dllexport) __stdcall load_math(vm_t *vm);
#else
void load_math(vm_t *vm);
#endif

#endif
