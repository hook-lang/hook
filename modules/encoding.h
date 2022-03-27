//
// Hook Programming Language
// encoding.h
//

#ifndef ENCODING_H
#define ENCODING_H

#include "hook.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_encoding(hk_vm_t *vm);
#else
int32_t load_encoding(hk_vm_t *vm);
#endif

#endif // ENCODING_H
