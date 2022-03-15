//
// Hook Programming Language
// hook_compiler.h
//

#ifndef HOOK_COMPILER_H
#define HOOK_COMPILER_H

#include "hook_callable.h"

hk_closure_t *hk_compile(hk_string_t *file, hk_string_t *source);

#endif // HOOK_COMPILER_H
