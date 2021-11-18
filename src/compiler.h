//
// Hook Programming Language
// compiler.h
//

#ifndef COMPILER_H
#define COMPILER_H

#include "callable.h"

function_t *compile(string_t *file, string_t *source);

#endif // COMPILER_H
