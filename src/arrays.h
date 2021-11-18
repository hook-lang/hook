//
// Hook Programming Language
// arrays.h
//

#ifndef ARRAYS_H
#define ARRAYS_H

#include "vm.h"

#ifdef _WIN32
void __declspec(dllexport) __stdcall load_arrays(vm_t *vm);
#else
void load_arrays(vm_t *vm);
#endif

#endif // ARRAYS_H
