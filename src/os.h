//
// Hook Programming Language
// os.h
//

#ifndef OS_H
#define OS_H

#include "vm.h"

#ifdef _WIN32
void __declspec(dllexport) __stdcall load_os(vm_t *vm);
#else
void load_os(vm_t *vm);
#endif

#endif // OS_H
