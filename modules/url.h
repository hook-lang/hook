//
// Hook Programming Language
// url.h
//

#ifndef URL_H
#define URL_H

#include "hook.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_url(vm_t *vm);
#else
int load_url(vm_t *vm);
#endif

#endif // URL_H
