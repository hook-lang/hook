//
// Hook Programming Language
// h_module.h
//

#ifndef H_MODULE_H
#define H_MODULE_H

#include "h_vm.h"

void init_module_cache(void);
void free_module_cache(void);
int load_module(vm_t *vm);

#endif // H_MODULE_H
