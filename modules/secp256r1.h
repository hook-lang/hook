//
// The Hook Programming Language
// secp256r1.h
//

#ifndef SECP256R1_H
#define SECP256R1_H

#include "hk_vm.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_secp256r1(hk_vm_t *vm);
#else
int32_t load_secp256r1(hk_vm_t *vm);
#endif

#endif // SECP256R1_H
