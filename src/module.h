//
// The Hook Programming Language
// module.h
//

#ifndef MODULE_H
#define MODULE_H

#include <hook/state.h>

void init_module_cache(void);
void free_module_cache(void);
int32_t load_module(hk_state_t *state);

#endif // MODULE_H
