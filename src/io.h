//
// Hook Programming Language
// io.h
//

#ifndef IO_H
#define IO_H

#include "vm.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_io(vm_t *vm);
#else
int load_io(vm_t *vm);
#endif

#endif // IO_H
