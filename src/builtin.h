//
// The Hook Programming Language
// builtin.h
//

#ifndef BUILTIN_H
#define BUILTIN_H

#include <hook/state.h>

void load_globals(hk_state_t *state);
int32_t num_globals(void);
int32_t lookup_global(int32_t length, char *chars);

#endif // BUILTIN_H
