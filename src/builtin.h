//
// Hook Programming Language
// builtin.h
//

#ifndef BUILTIN_H
#define BUILTIN_H

#include "vm.h"

void load_globals(vm_t *vm);
int num_globals(void);
int lookup_global(int length, char *chars);

#endif
