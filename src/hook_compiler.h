//
// Hook Programming Language
// hook_compiler.h
//

#ifndef HOOK_COMPILER_H
#define HOOK_COMPILER_H

#include "hook_callable.h"

closure_t *compile(string_t *file, string_t *source);

#endif // HOOK_COMPILER_H
