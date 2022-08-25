//
// The Hook Programming Language
// mysql.h
//

#ifndef MYSQL_H
#define MYSQL_H

#include "hk_vm.h"

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_mysql(hk_vm_t *vm);
#else
int32_t load_mysql(hk_vm_t *vm);
#endif

#endif // MYSQL_H
