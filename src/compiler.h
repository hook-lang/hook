//
// Hook Programming Language
// compiler.h
//

#ifndef COMPILER_H
#define COMPILER_H

#include "scanner.h"
#include "chunk.h"
#include "array.h"

void compile(chunk_t *chunk, array_t *consts, scanner_t *scan);

#endif
