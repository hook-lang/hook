//
// Hook Programming Language
// hook_builtin.h
//

#ifndef HOOK_BUILTIN_H
#define HOOK_BUILTIN_H

#include "hook_vm.h"

void load_globals(vm_t *vm);
int num_globals(void);
int lookup_global(int length, char *chars);

#endif // HOOK_BUILTIN_H
