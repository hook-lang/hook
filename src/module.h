//
// Hook Programming Language
// module.h
//

#ifndef MODULE_H
#define MODULE_H

#include "vm.h"

void init_module_cache(void);
void free_module_cache(void);
int load_module(vm_t *vm);

#endif // MODULE_H
