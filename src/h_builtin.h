//
// Hook Programming Language
// h_builtin.h
//

#ifndef H_BUILTIN_H
#define H_BUILTIN_H

#include "h_vm.h"

void load_globals(vm_t *vm);
int num_globals(void);
int lookup_global(int length, char *chars);

#endif // H_BUILTIN_H
