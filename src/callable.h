//
// Hook Programming Language
// callable.h
//

#ifndef CALLABLE_H
#define CALLABLE_H

#include "string.h"
#include "chunk.h"
#include "array.h"

typedef struct
{
  OBJECT_HEADER
  int arity;
  string_t *name;
  chunk_t chunk;
  array_t *consts;
} function_t;

function_t *function_new(string_t *name, int arity);
void function_free(function_t *fn);

#endif
