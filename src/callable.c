//
// Hook Programming Language
// callable.c
//

#include "callable.h"
#include <stdlib.h>
#include "common.h"
#include "memory.h"

#define MIN_CAPACITY 8

static inline void resize(function_t *fn);

static inline void resize(function_t *fn)
{
  if (fn->length < fn->capacity)
    return;
  int capacity = fn->capacity << 1;
  fn->capacity = capacity;
  fn->lines = (line_t *) reallocate(fn->lines,
    sizeof(*fn->lines) * capacity);
}

function_t *function_new(int arity, string_t *name, string_t *file)
{
  function_t *fn = (function_t *) allocate(sizeof(*fn));
  fn->ref_count = 0;
  fn->arity = arity;
  INCR_REF(name);
  fn->name = name;
  INCR_REF(file);
  fn->file = file;
  fn->capacity = MIN_CAPACITY;
  fn->length = 0;
  fn->lines = (line_t *) allocate(sizeof(*fn->lines) * fn->capacity);
  chunk_init(&fn->chunk);
  fn->consts = array_allocate(0);
  fn->consts->length = 0;
  return fn;
}

void function_free(function_t *fn)
{
  string_t *name = fn->name;
  DECR_REF(name);
  if (IS_UNREACHABLE(name))
    string_free(name);
  string_t *file = fn->file;
  DECR_REF(file);
  if (IS_UNREACHABLE(file))
    string_free(file);
  free(fn->lines);
  chunk_free(&fn->chunk);
  array_free(fn->consts);
  free(fn);
}

void function_add_line(function_t *fn, int line_no)
{
  resize(fn);
  line_t *line = &fn->lines[fn->length];
  line->no = line_no;
  line->offset = fn->chunk.length;
  ++fn->length;
}

int function_get_line(function_t *fn, int offset)
{
  int line_no = -1;
  line_t *lines = fn->lines;
  for (int i = 0; i < fn->length; ++i)
  {
    line_t *line = &lines[i];
    if (line->offset == offset)
    {
      line_no = line->no;
      break;
    }
  }
  ASSERT(line_no != -1, "function must be contains the line number");
  return line_no;
}

native_t *native_new(string_t *name, int arity, int (*call)(struct vm *, value_t *))
{
  native_t *native = (native_t *) allocate(sizeof(*native));
  native->ref_count = 0;
  native->arity = arity;
  INCR_REF(name);
  native->name = name;
  native->call = call;
  return native;
}

void native_free(native_t *native)
{
  string_t *name = native->name;
  DECR_REF(name);
  if (IS_UNREACHABLE(name))
    string_free(name);
  free(native);
}
