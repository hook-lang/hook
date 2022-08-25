//
// The Hook Programming Language
// hk_arrays.h
//

#ifndef HK_ARRAYS_H
#define HK_ARRAYS_H

#include "hk_vm.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_arrays(hk_vm_t *vm);
#else
int32_t load_arrays(hk_vm_t *vm);
#endif

#endif // HK_ARRAYS_H
