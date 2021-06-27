//
// Hook Programming Language
// compiler.h
//

#ifndef COMPILER_H
#define COMPILER_H

#include "scanner.h"
#include "chunk.h"
#include "array.h"

#define COMPILER_MAX_LOCALS 256

typedef struct
{
  int length;
  char *start;
} local_t;

typedef struct
{
  scanner_t *scan;
  chunk_t *chunk;
  array_t *consts;
  local_t locals[COMPILER_MAX_LOCALS];
  int local_count;
} compiler_t;

void compiler_init(compiler_t *comp, chunk_t *chunk, array_t *consts, scanner_t *scan);
void compiler_compile(compiler_t *comp);

#endif
