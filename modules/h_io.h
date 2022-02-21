//
// Hook Programming Language
// h_io.h
//

#ifndef H_IO_H
#define H_IO_H

#include "h_vm.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_io(vm_t *vm);
#else
int load_io(vm_t *vm);
#endif

#endif // H_IO_H
