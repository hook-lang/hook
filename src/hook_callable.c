//
// Hook Programming Language
// hook_callable.c
//

#include "hook_callable.h"
#include <stdlib.h>
#include "hook_common.h"
#include "hook_memory.h"

#define MIN_CAPACITY (1 << 3)

function_t *function_allocate(int arity, string_t *name, string_t *file);
static inline void init_lines(function_t *fn);
static inline void init_functions(function_t *fn);
static inline void free_functions(function_t *fn);
static inline void resize_lines(function_t *fn);
static inline void resize_functions(function_t *fn);

function_t *function_allocate(int arity, string_t *name, string_t *file)
{
  function_t *fn = (function_t *) allocate(sizeof(*fn));
  fn->ref_count = 0;
  fn->arity = arity;
  if (name)
    INCR_REF(name);
  fn->name = name;
  INCR_REF(file);
  fn->file = file;
  return fn;
}

static inline void init_lines(function_t *fn)
{
  fn->lines_capacity = MIN_CAPACITY;
  fn->num_lines = 0;
  fn->lines = (line_t *) allocate(sizeof(*fn->lines) * fn->lines_capacity);
}

static inline void init_functions(function_t *fn)
{
  fn->functions_capacity = MIN_CAPACITY;
  fn->num_functions = 0;
  fn->functions = (function_t **) allocate(sizeof(*fn->functions) * fn->functions_capacity);
}

static inline void free_functions(function_t *fn)
{
  for (int i = 0; i < fn->num_functions; ++i)
    function_release(fn->functions[i]);
  free(fn->functions);
}

static inline void resize_lines(function_t *fn)
{
  if (fn->num_lines < fn->lines_capacity)
    return;
  int capacity = fn->lines_capacity << 1;
  fn->lines_capacity = capacity;
  fn->lines = (line_t *) reallocate(fn->lines,
    sizeof(*fn->lines) * capacity);
}

static inline void resize_functions(function_t *fn)
{
  if (fn->num_functions < fn->functions_capacity)
    return;
  uint8_t capacity = fn->functions_capacity << 1;
  fn->functions_capacity = capacity;
  fn->functions = (function_t **) reallocate(fn->functions,
    sizeof(*fn->functions) * capacity);
}

function_t *function_new(int arity, string_t *name, string_t *file)
{
  function_t *fn = function_allocate(arity, name, file);
  init_lines(fn);
  chunk_init(&fn->chunk);
  fn->consts = array_allocate(0);
  fn->consts->length = 0;
  init_functions(fn);
  fn->num_nonlocals = 0;
  return fn;
}

void function_free(function_t *fn)
{
  string_t *name = fn->name;
  if (name)
    string_release(name);
  string_release(fn->file);
  free(fn->lines);
  chunk_free(&fn->chunk);
  array_free(fn->consts);
  free_functions(fn);
  free(fn);
}

void function_release(function_t *fn)
{
  DECR_REF(fn);
  if (IS_UNREACHABLE(fn))
    function_free(fn);
}

void function_add_line(function_t *fn, int line_no)
{
  resize_lines(fn);
  line_t *line = &fn->lines[fn->num_lines];
  line->no = line_no;
  line->offset = fn->chunk.length;
  ++fn->num_lines;
}

int function_get_line(function_t *fn, int offset)
{
  int line_no = -1;
  line_t *lines = fn->lines;
  for (int i = 0; i < fn->num_lines; ++i)
  {
    line_t *line = &lines[i];
    if (line->offset == offset)
    {
      line_no = line->no;
      break;
    }
  }
  ASSERT(line_no != -1, "function must contain the line number");
  return line_no;
}

void function_add_child(function_t *fn, function_t *child)
{
  resize_functions(fn);
  INCR_REF(child);
  fn->functions[fn->num_functions] = child;
  ++fn->num_functions;
}

void function_serialize(function_t *fn, FILE *stream)
{
  fwrite(&fn->arity, sizeof(fn->arity), 1, stream);
  string_serialize(fn->name, stream);
  string_serialize(fn->file, stream);
  fwrite(&fn->lines_capacity, sizeof(fn->lines_capacity), 1, stream);
  fwrite(&fn->num_lines, sizeof(fn->num_lines), 1, stream);
  for (int i = 0; i < fn->num_lines; ++i)
  {
    line_t *line = &fn->lines[i];
    fwrite(line, sizeof(*line), 1, stream);
  }
  chunk_serialize(&fn->chunk, stream);
  array_serialize(fn->consts, stream);
  fwrite(&fn->functions_capacity, sizeof(fn->functions_capacity), 1, stream);
  fwrite(&fn->num_functions, sizeof(fn->num_functions), 1, stream);
  function_t **functions = fn->functions;
  for (int i = 0; i < fn->num_functions; ++i)
    function_serialize(functions[i], stream);
  fwrite(&fn->num_nonlocals, sizeof(fn->num_nonlocals), 1, stream);
}

function_t *function_deserialize(FILE *stream)
{
  int arity;
  if (fread(&arity, sizeof(arity), 1, stream) != 1)
    return NULL;
  string_t *name = string_deserialize(stream);
  if (!name)
    return NULL;
  string_t *file = string_deserialize(stream);
  if (!file)
    return NULL;
  function_t *fn = function_allocate(arity, name, file);
  if (fread(&fn->lines_capacity, sizeof(fn->lines_capacity), 1, stream) != 1)
    return NULL;
  if (fread(&fn->num_lines, sizeof(fn->num_lines), 1, stream) != 1)
    return NULL;
  fn->lines = (line_t *) allocate(sizeof(*fn->lines) * fn->lines_capacity);
  for (int i = 0; i < fn->num_lines; ++i)
  {
    line_t *line = &fn->lines[i];
    if (fread(line, sizeof(*line), 1, stream) != 1)
      return NULL;
  }
  if (!chunk_deserialize(&fn->chunk, stream))
    return NULL;
  fn->consts = array_deserialize(stream);
  if (!fn->consts)
    return NULL;
  INCR_REF(fn->consts);
  if (fread(&fn->functions_capacity, sizeof(fn->functions_capacity), 1, stream) != 1)
    return NULL;
  if (fread(&fn->num_functions, sizeof(fn->num_functions), 1, stream) != 1)
    return NULL;
  function_t **functions = (function_t **) allocate(sizeof(*functions) * fn->functions_capacity);
  for (int i = 0; i < fn->num_functions; ++i)
  {
    function_t *fn = function_deserialize(stream);
    if (!fn)
      return NULL;
    INCR_REF(fn);
    functions[i] = fn;
  }
  fn->functions = functions;
  if (fread(&fn->num_nonlocals, sizeof(fn->num_nonlocals), 1, stream) != 1)
    return NULL;
  return fn;
}

closure_t *closure_new(function_t *fn)
{
  int size = sizeof(closure_t) + sizeof(value_t) * fn->num_nonlocals;
  closure_t *cl = (closure_t *) allocate(size);
  cl->ref_count = 0;
  INCR_REF(fn);
  cl->fn = fn;
  return cl;
}

void closure_free(closure_t *cl)
{
  function_t *fn = cl->fn;
  int num_nonlocals = fn->num_nonlocals;
  function_release(fn);
  for (int i = 0; i < num_nonlocals; ++i)
    value_release(cl->nonlocals[i]);
  free(cl);
}

void closure_release(closure_t *cl)
{
  DECR_REF(cl);
  if (IS_UNREACHABLE(cl))
    closure_free(cl);
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
  string_release(native->name);
  free(native);
}

void native_release(native_t *native)
{
  DECR_REF(native);
  if (IS_UNREACHABLE(native))
    native_free(native);
}
