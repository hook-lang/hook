//
// Hook Programming Language
// encoding.h
//

#ifndef ENCODING_H
#define ENCODING_H

#include "h_vm.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_encoding(vm_t *vm);
#else
int load_encoding(vm_t *vm);
#endif

#endif // ENCODING_H
