//
// Hook Programming Language
// hashing.h
//

#ifndef HASHING_H
#define HASHING_H

#include "hook.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_hashing(hk_vm_t *vm);
#else
int32_t load_hashing(hk_vm_t *vm);
#endif

#endif // HASHING_H
