//
// Hook Programming Language
// h_strings.h
//

#ifndef H_STRINGS_H
#define H_STRINGS_H

#include "h_vm.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_strings(vm_t *vm);
#else
int load_strings(vm_t *vm);
#endif

#endif // H_STRINGS_H
