//
// Hook Programming Language
// hook_builtin.h
//

#ifndef HOOK_BUILTIN_H
#define HOOK_BUILTIN_H

#include "hook_vm.h"

void load_globals(hk_vm_t *vm);
int32_t num_globals(void);
int32_t lookup_global(int32_t length, char *chars);

#endif // HOOK_BUILTIN_H
