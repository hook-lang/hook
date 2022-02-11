//
// Hook Programming Language
// secp256r1.h
//

#ifndef SECP256R1_H
#define SECP256R1_H

#include "vm.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_secp256r1(vm_t *vm);
#else
int load_secp256r1(vm_t *vm);
#endif

#endif // SECP256R1_H
