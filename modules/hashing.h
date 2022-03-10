//
// Hook Programming Language
// hashing.h
//

#ifndef HASHING_H
#define HASHING_H

#include "hook.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_hashing(vm_t *vm);
#else
int load_hashing(vm_t *vm);
#endif

#endif // HASHING_H
