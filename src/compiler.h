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
#define COMPILER_MAX_BREAKS 256

typedef struct
{
  int depth;
  int length;
  char *start;
} local_t;

typedef struct loop
{
  struct loop *enclosing;
  int scope_depth;
  int jump;
  int num_offsets;
  int offsets[COMPILER_MAX_BREAKS];
} loop_t;

typedef struct
{
  scanner_t *scan;
  chunk_t *chunk;
  array_t *consts;
  int scope_depth;
  int num_locals;
  local_t locals[COMPILER_MAX_LOCALS];
  loop_t *loop;
} compiler_t;

void compiler_init(compiler_t *comp, chunk_t *chunk, array_t *consts, scanner_t *scan);
void compiler_compile(compiler_t *comp);

#endif
