//
// Hook Programming Language
// compiler.h
//

#ifndef COMPILER_H
#define COMPILER_H

#include "scanner.h"
#include "chunk.h"

void compile(chunk_t *chunk, scanner_t *scan);

#endif
