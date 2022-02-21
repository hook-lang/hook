//
// Hook Programming Language
// h_os.h
//

#ifndef H_OS_H
#define H_OS_H

#include "h_vm.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_os(vm_t *vm);
#else
int load_os(vm_t *vm);
#endif

#endif // H_OS_H
