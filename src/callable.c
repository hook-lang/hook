//
// Hook Programming Language
// callable.c
//

#include "callable.h"
#include <stdlib.h>
#include "memory.h"

function_t *function_new(string_t *name, int arity)
{
  function_t *fn = (function_t *) allocate(sizeof(*fn));
  fn->ref_count = 0;
  fn->arity = arity;
  INCR_REF(name);
  fn->name = name;
  chunk_init(&fn->chunk, 0);
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
  chunk_free(&fn->chunk);
  array_free(fn->consts);
  free(fn);
}
