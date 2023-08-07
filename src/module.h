//
// The Hook Programming Language
// module.h
//

#ifndef MODULE_H
#define MODULE_H

#include <hook/state.h>

void module_cache_init(void);
void module_cache_deinit(void);
void module_load(HkState *state, HkString *currFile);

#endif // MODULE_H
