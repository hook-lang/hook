//
// The Hook Programming Language
// hk_os.h
//

#ifndef HK_OS_H
#define HK_OS_H

#include "hk_vm.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_os(hk_vm_t *vm);
#else
int32_t load_os(hk_vm_t *vm);
#endif

#endif // HK_OS_H
