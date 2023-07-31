//
// The Hook Programming Language
// builtin.h
//

#ifndef BUILTIN_H
#define BUILTIN_H

#include <hook/state.h>

void load_globals(HkState *state);
int num_globals(void);
int lookup_global(int length, char *chars);

#endif // BUILTIN_H
