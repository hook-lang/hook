//
// Hook Programming Language
// h_compiler.h
//

#ifndef H_COMPILER_H
#define H_COMPILER_H

#include "h_callable.h"

closure_t *compile(string_t *file, string_t *source);

#endif // H_COMPILER_H
