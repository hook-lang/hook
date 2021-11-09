//
// Hook Programming Language
// url.h
//

#ifndef URL_H
#define URL_H

#include "vm.h"

#ifdef WIN32
void __declspec(dllexport) __stdcall load_url(vm_t *vm);
#else
void load_url(vm_t *vm);
#endif

#endif
