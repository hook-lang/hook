//
// Hook Programming Language
// callable.h
//

#ifndef CALLABLE_H
#define CALLABLE_H

#include "string.h"
#include "chunk.h"
#include "array.h"

#define CALLABLE_HEADER OBJECT_HEADER \
                        int arity; \
                        string_t *name;

typedef struct
{
  CALLABLE_HEADER
} callable_t;

typedef struct
{
  CALLABLE_HEADER
  chunk_t chunk;
  array_t *consts;
} function_t;

struct vm;

typedef struct
{
  CALLABLE_HEADER
  void (*call)(struct vm *vm, value_t *frame);
} native_t;

function_t *function_new(string_t *name, int arity);
void function_free(function_t *fn);
native_t *native_new(string_t *name, int arity, void (*call)(struct vm *, value_t *));
void native_free(native_t *native);

#endif
