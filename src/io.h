//
// Hook Programming Language
// io.h
//

#ifndef IO_H
#define IO_H

#include "vm.h"

#ifdef _WIN32
void __declspec(dllexport) __stdcall load_io(vm_t *vm);
#else
void load_io(vm_t *vm);
#endif

#endif // IO_H
