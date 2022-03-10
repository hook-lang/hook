//
// Hook Programming Language
// sqlite.h
//

#ifndef SQLITE_H
#define SQLITE_H

#include "hook.h"

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_sqlite(vm_t *vm);
#else
int load_sqlite(vm_t *vm);
#endif

#endif // SQLITE_H
