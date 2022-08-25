//
// The Hook Programming Language
// hk_callable.c
//

#include "hk_callable.h"
#include <stdlib.h>
#include "hk_utils.h"
#include "hk_memory.h"

#define MIN_CAPACITY (1 << 3)

static inline hk_function_t *function_allocate(int32_t arity, hk_string_t *name, hk_string_t *file);
static inline void init_functions(hk_function_t *fn);
static inline void free_functions(hk_function_t *fn);
static inline void grow_functions(hk_function_t *fn);

static inline hk_function_t *function_allocate(int32_t arity, hk_string_t *name, hk_string_t *file)
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

static inline void init_functions(hk_function_t *fn)
{
  fn->functions_capacity = MIN_CAPACITY;
  fn->functions_length = 0;
  fn->functions = (hk_function_t **) hk_allocate(sizeof(*fn->functions) * fn->functions_capacity);
}

static inline void free_functions(hk_function_t *fn)
{
  for (int32_t i = 0; i < fn->functions_length; ++i)
    hk_function_release(fn->functions[i]);
  free(fn->functions);
}

static inline void grow_functions(hk_function_t *fn)
{
  if (fn->functions_length < fn->functions_capacity)
    return;
  uint8_t capacity = fn->functions_capacity << 1;
  fn->functions_capacity = capacity;
  fn->functions = (hk_function_t **) hk_reallocate(fn->functions,
    sizeof(*fn->functions) * capacity);
}

hk_function_t *hk_function_new(int32_t arity, hk_string_t *name, hk_string_t *file)
{
  hk_function_t *fn = function_allocate(arity, name, file);
  hk_chunk_init(&fn->chunk);
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
  hk_chunk_free(&fn->chunk);
  free_functions(fn);
  free(fn);
}

void hk_function_release(hk_function_t *fn)
{
  hk_decr_ref(fn);
  if (hk_is_unreachable(fn))
    hk_function_free(fn);
}

void hk_function_add_child(hk_function_t *fn, hk_function_t *child)
{
  grow_functions(fn);
  hk_incr_ref(child);
  fn->functions[fn->functions_length] = child;
  ++fn->functions_length;
}

void hk_function_serialize(hk_function_t *fn, FILE *stream)
{
  fwrite(&fn->arity, sizeof(fn->arity), 1, stream);
  hk_string_serialize(fn->name, stream);
  hk_string_serialize(fn->file, stream);
  hk_chunk_serialize(&fn->chunk, stream);
  fwrite(&fn->functions_capacity, sizeof(fn->functions_capacity), 1, stream);
  fwrite(&fn->functions_length, sizeof(fn->functions_length), 1, stream);
  hk_function_t **functions = fn->functions;
  for (int32_t i = 0; i < fn->functions_length; ++i)
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
  if (!hk_chunk_deserialize(&fn->chunk, stream))
    return NULL;
  if (fread(&fn->functions_capacity, sizeof(fn->functions_capacity), 1, stream) != 1)
    return NULL;
  if (fread(&fn->functions_length, sizeof(fn->functions_length), 1, stream) != 1)
    return NULL;
  hk_function_t **functions = (hk_function_t **) hk_allocate(sizeof(*functions) * fn->functions_capacity);
  for (int32_t i = 0; i < fn->functions_length; ++i)
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
  int32_t size = sizeof(hk_closure_t) + sizeof(hk_value_t) * (fn->num_nonlocals - 1);
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
