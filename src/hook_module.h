//
// Hook Programming Language
// hook_module.h
//

#ifndef HOOK_MODULE_H
#define HOOK_MODULE_H

#include "hook_vm.h"

void init_module_cache(void);
void free_module_cache(void);
int load_module(hk_vm_t *vm);

#endif // HOOK_MODULE_H
