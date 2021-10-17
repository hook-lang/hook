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
  int no;
  int offset;
} line_t;

typedef struct
{
  CALLABLE_HEADER
  string_t *file;
  int capacity;
  int length;
  line_t *lines;
  chunk_t chunk;
  array_t *consts;
} function_t;

struct vm;

typedef struct
{
  CALLABLE_HEADER
  int (*call)(struct vm *vm, value_t *frame);
} native_t;

function_t *function_new(int arity, string_t *name, string_t *file);
void function_free(function_t *fn);
void function_add_line(function_t *fn, int line_no);
int function_get_line(function_t *fn, int offset);
native_t *native_new(string_t *name, int arity, int (*call)(struct vm *, value_t *));
void native_free(native_t *native);

#endif
