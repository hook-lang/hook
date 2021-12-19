//
// Hook Programming Language
// strings.h
//

#ifndef STRINGS_H
#define STRINGS_H

#include "vm.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_strings(vm_t *vm);
#else
int load_strings(vm_t *vm);
#endif

#endif // STRINGS_H
