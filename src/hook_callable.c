//
// Hook Programming Language
// hook_callable.c
//

#include "hook_callable.h"
#include <stdlib.h>
#include "hook_utils.h"
#include "hook_memory.h"

#define MIN_CAPACITY (1 << 3)

hk_function_t *function_allocate(int32_t arity, hk_string_t *name, hk_string_t *file);
static inline void init_lines(hk_function_t *fn);
static inline void init_functions(hk_function_t *fn);
static inline void free_functions(hk_function_t *fn);
static inline void grow_lines(hk_function_t *fn);
static inline void grow_functions(hk_function_t *fn);

hk_function_t *function_allocate(int32_t arity, hk_string_t *name, hk_string_t *file)
{
  hk_function_t *fn = (hk_function_t *) hk_allocate(sizeof(*fn));
  fn->ref_count = 0;
  fn->arity = arity;
  if (name)
    hk_incr_ref(name);
  fn->name = name;
  hk_incr_ref(file);
  fn->file = file;
  return fn;
}

static inline void init_lines(hk_function_t *fn)
{
  fn->lines_capacity = MIN_CAPACITY;
  fn->num_lines = 0;
  fn->lines = (hk_line_t *) hk_allocate(sizeof(*fn->lines) * fn->lines_capacity);
}

static inline void init_functions(hk_function_t *fn)
{
  fn->functions_capacity = MIN_CAPACITY;
  fn->num_functions = 0;
  fn->functions = (hk_function_t **) hk_allocate(sizeof(*fn->functions) * fn->functions_capacity);
}

static inline void free_functions(hk_function_t *fn)
{
  for (int32_t i = 0; i < fn->num_functions; ++i)
    hk_function_release(fn->functions[i]);
  free(fn->functions);
}

static inline void grow_lines(hk_function_t *fn)
{
  if (fn->num_lines < fn->lines_capacity)
    return;
  int32_t capacity = fn->lines_capacity << 1;
  fn->lines_capacity = capacity;
  fn->lines = (hk_line_t *) hk_reallocate(fn->lines,
    sizeof(*fn->lines) * capacity);
}

static inline void grow_functions(hk_function_t *fn)
{
  if (fn->num_functions < fn->functions_capacity)
    return;
  uint8_t capacity = fn->functions_capacity << 1;
  fn->functions_capacity = capacity;
  fn->functions = (hk_function_t **) hk_reallocate(fn->functions,
    sizeof(*fn->functions) * capacity);
}

hk_function_t *hk_function_new(int32_t arity, hk_string_t *name, hk_string_t *file)
{
  hk_function_t *fn = function_allocate(arity, name, file);
  init_lines(fn);
  hk_chunk_init(&fn->chunk);
  fn->consts = hk_array_new();
  init_functions(fn);
  fn->num_nonlocals = 0;
  return fn;
}

void hk_function_free(hk_function_t *fn)
{
  hk_string_t *name = fn->name;
  if (name)
    hk_string_release(name);
  hk_string_release(fn->file);
  free(fn->lines);
  hk_chunk_free(&fn->chunk);
  hk_array_free(fn->consts);
  free_functions(fn);
  free(fn);
}

void hk_function_release(hk_function_t *fn)
{
  hk_decr_ref(fn);
  if (hk_is_unreachable(fn))
    hk_function_free(fn);
}

void hk_function_add_line(hk_function_t *fn, int32_t line_no)
{
  grow_lines(fn);
  hk_line_t *line = &fn->lines[fn->num_lines];
  line->no = line_no;
  line->offset = fn->chunk.length;
  ++fn->num_lines;
}

int32_t hk_function_get_line(hk_function_t *fn, int32_t offset)
{
  int32_t line_no = -1;
  hk_line_t *lines = fn->lines;
  for (int32_t i = 0; i < fn->num_lines; ++i)
  {
    hk_line_t *line = &lines[i];
    if (line->offset == offset)
    {
      line_no = line->no;
      break;
    }
  }
  hk_assert(line_no != -1, "function must contain the line number");
  return line_no;
}

void hk_function_add_child(hk_function_t *fn, hk_function_t *child)
{
  grow_functions(fn);
  hk_incr_ref(child);
  fn->functions[fn->num_functions] = child;
  ++fn->num_functions;
}

void hk_function_serialize(hk_function_t *fn, FILE *stream)
{
  fwrite(&fn->arity, sizeof(fn->arity), 1, stream);
  hk_string_serialize(fn->name, stream);
  hk_string_serialize(fn->file, stream);
  fwrite(&fn->lines_capacity, sizeof(fn->lines_capacity), 1, stream);
  fwrite(&fn->num_lines, sizeof(fn->num_lines), 1, stream);
  for (int32_t i = 0; i < fn->num_lines; ++i)
  {
    hk_line_t *line = &fn->lines[i];
    fwrite(line, sizeof(*line), 1, stream);
  }
  hk_chunk_serialize(&fn->chunk, stream);
  hk_array_serialize(fn->consts, stream);
  fwrite(&fn->functions_capacity, sizeof(fn->functions_capacity), 1, stream);
  fwrite(&fn->num_functions, sizeof(fn->num_functions), 1, stream);
  hk_function_t **functions = fn->functions;
  for (int32_t i = 0; i < fn->num_functions; ++i)
    hk_function_serialize(functions[i], stream);
  fwrite(&fn->num_nonlocals, sizeof(fn->num_nonlocals), 1, stream);
}

hk_function_t *hk_function_deserialize(FILE *stream)
{
  int32_t arity;
  if (fread(&arity, sizeof(arity), 1, stream) != 1)
    return NULL;
  hk_string_t *name = hk_string_deserialize(stream);
  if (!name)
    return NULL;
  hk_string_t *file = hk_string_deserialize(stream);
  if (!file)
    return NULL;
  hk_function_t *fn = function_allocate(arity, name, file);
  if (fread(&fn->lines_capacity, sizeof(fn->lines_capacity), 1, stream) != 1)
    return NULL;
  if (fread(&fn->num_lines, sizeof(fn->num_lines), 1, stream) != 1)
    return NULL;
  fn->lines = (hk_line_t *) hk_allocate(sizeof(*fn->lines) * fn->lines_capacity);
  for (int32_t i = 0; i < fn->num_lines; ++i)
  {
    hk_line_t *line = &fn->lines[i];
    if (fread(line, sizeof(*line), 1, stream) != 1)
      return NULL;
  }
  if (!hk_chunk_deserialize(&fn->chunk, stream))
    return NULL;
  fn->consts = hk_array_deserialize(stream);
  if (!fn->consts)
    return NULL;
  hk_incr_ref(fn->consts);
  if (fread(&fn->functions_capacity, sizeof(fn->functions_capacity), 1, stream) != 1)
    return NULL;
  if (fread(&fn->num_functions, sizeof(fn->num_functions), 1, stream) != 1)
    return NULL;
  hk_function_t **functions = (hk_function_t **) hk_allocate(sizeof(*functions) * fn->functions_capacity);
  for (int32_t i = 0; i < fn->num_functions; ++i)
  {
    hk_function_t *fn = hk_function_deserialize(stream);
    if (!fn)
      return NULL;
    hk_incr_ref(fn);
    functions[i] = fn;
  }
  fn->functions = functions;
  if (fread(&fn->num_nonlocals, sizeof(fn->num_nonlocals), 1, stream) != 1)
    return NULL;
  return fn;
}

hk_closure_t *hk_closure_new(hk_function_t *fn)
{
  int32_t size = sizeof(hk_closure_t) + sizeof(hk_value_t) * fn->num_nonlocals;
  hk_closure_t *cl = (hk_closure_t *) hk_allocate(size);
  cl->ref_count = 0;
  hk_incr_ref(fn);
  cl->fn = fn;
  return cl;
}

void hk_closure_free(hk_closure_t *cl)
{
  hk_function_t *fn = cl->fn;
  int32_t num_nonlocals = fn->num_nonlocals;
  hk_function_release(fn);
  for (int32_t i = 0; i < num_nonlocals; ++i)
    hk_value_release(cl->nonlocals[i]);
  free(cl);
}

void hk_closure_release(hk_closure_t *cl)
{
  hk_decr_ref(cl);
  if (hk_is_unreachable(cl))
    hk_closure_free(cl);
}

hk_native_t *hk_native_new(hk_string_t *name, int32_t arity, int32_t (*call)(struct hk_vm *, hk_value_t *))
{
  hk_native_t *native = (hk_native_t *) hk_allocate(sizeof(*native));
  native->ref_count = 0;
  native->arity = arity;
  hk_incr_ref(name);
  native->name = name;
  native->call = call;
  return native;
}

void hk_native_free(hk_native_t *native)
{
  hk_string_release(native->name);
  free(native);
}

void hk_native_release(hk_native_t *native)
{
  hk_decr_ref(native);
  if (hk_is_unreachable(native))
    hk_native_free(native);
}
