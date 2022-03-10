//
// Hook Programming Language
// hook_callable.h
//

#ifndef HOOK_CALLABLE_H
#define HOOK_CALLABLE_H

#include "hook_string.h"
#include "hook_chunk.h"
#include "hook_array.h"

typedef struct
{
  int no;
  int offset;
} line_t;

typedef struct function
{
  OBJECT_HEADER
  int arity;
  string_t *name;
  string_t *file;
  int lines_capacity;
  int num_lines;
  line_t *lines;
  chunk_t chunk;
  array_t *consts;
  uint8_t functions_capacity;
  uint8_t num_functions;
  struct function **functions;
  uint8_t num_nonlocals;
} function_t;

typedef struct
{
  OBJECT_HEADER
  function_t *fn;
  value_t nonlocals[0];
} closure_t;

struct vm;

typedef struct
{
  OBJECT_HEADER
  int arity;
  string_t *name;
  int (*call)(struct vm *, value_t *);
} native_t;

function_t *function_new(int arity, string_t *name, string_t *file);
void function_free(function_t *fn);
void function_release(function_t *fn);
void function_add_line(function_t *fn, int line_no);
int function_get_line(function_t *fn, int offset);
void function_add_child(function_t *fn, function_t *child);
void function_serialize(function_t *fn, FILE *stream);
function_t *function_deserialize(FILE *stream);
closure_t *closure_new(function_t *fn);
void closure_free(closure_t *cl);
void closure_release(closure_t *cl);
native_t *native_new(string_t *name, int arity, int (*call)(struct vm *, value_t *));
void native_free(native_t *native);
void native_release(native_t *native);

#endif // HOOK_CALLABLE_H
