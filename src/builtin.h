//
// Hook Programming Language
// builtin.h
//

#ifndef BUILTIN_H
#define BUILTIN_H

#include "vm.h"

void globals_init(vm_t *vm);
int lookup_global(int length, char *chars);

#endif
