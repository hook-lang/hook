//
// The Hook Programming Language
// hk_module.h
//

#ifndef HK_MODULE_H
#define HK_MODULE_H

#include "hk_vm.h"

void init_module_cache(void);
void free_module_cache(void);
int32_t load_module(hk_vm_t *vm);

#endif // HK_MODULE_H
