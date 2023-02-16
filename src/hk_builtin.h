//
// The Hook Programming Language
// hk_builtin.h
//

#ifndef HK_BUILTIN_H
#define HK_BUILTIN_H

#include "hk_state.h"

void load_globals(hk_state_t *state);
int32_t num_globals(void);
int32_t lookup_global(int32_t length, char *chars);

#endif // HK_BUILTIN_H
