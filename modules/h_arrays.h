//
// Hook Programming Language
// h_arrays.h
//

#ifndef H_ARRAYS_H
#define H_ARRAYS_H

#include "h_vm.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_arrays(vm_t *vm);
#else
int load_arrays(vm_t *vm);
#endif

#endif // H_ARRAYS_H
