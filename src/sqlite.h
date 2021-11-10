//
// Hook Programming Language
// sqlite.h
//

#ifndef SQLITE_H
#define SQLITE_H

#include "vm.h"

#ifdef _WIN32
void __declspec(dllexport) __stdcall load_sqlite(vm_t *vm);
#else
void load_sqlite(vm_t *vm);
#endif

#endif
