//
// The Hook Programming Language
// sqlite.h
//

#ifndef SQLITE_H
#define SQLITE_H

#include "hk_vm.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_sqlite(hk_vm_t *vm);
#else
int32_t load_sqlite(hk_vm_t *vm);
#endif

#endif // SQLITE_H
