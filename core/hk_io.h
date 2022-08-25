//
// The Hook Programming Language
// hk_io.h
//

#ifndef HK_IO_H
#define HK_IO_H

#include "hk_vm.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_io(hk_vm_t *vm);
#else
int32_t load_io(hk_vm_t *vm);
#endif

#endif // HK_IO_H
