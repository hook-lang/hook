//
// The Hook Programming Language
// callable.c
//

#include <hook/callable.h>
#include <stdlib.h>
#include <hook/memory.h>
#include <hook/utils.h>

#define MIN_CAPACITY (1 << 3)

static inline HkFunction *function_allocate(int arity, HkString *name, HkString *file);
static inline void init_functions(HkFunction *fn);
static inline void free_functions(HkFunction *fn);
static inline void grow_functions(HkFunction *fn);

static inline HkFunction *function_allocate(int arity, HkString *name, HkString *file)
{
  HkFunction *fn = (HkFunction *) hk_allocate(sizeof(*fn));
  fn->ref_count = 0;
  fn->arity = arity;
  if (name)
    hk_incr_ref(name);
  fn->name = name;
  hk_incr_ref(file);
  fn->file = file;
  return fn;
}

static inline void init_functions(HkFunction *fn)
{
  fn->functions_capacity = MIN_CAPACITY;
  fn->functions_length = 0;
  fn->functions = (HkFunction **) hk_allocate(sizeof(*fn->functions) * fn->functions_capacity);
}

static inline void free_functions(HkFunction *fn)
{
  for (int i = 0; i < fn->functions_length; ++i)
    hk_function_release(fn->functions[i]);
  free(fn->functions);
}

static inline void grow_functions(HkFunction *fn)
{
  if (fn->functions_length < fn->functions_capacity)
    return;
  uint8_t capacity = fn->functions_capacity << 1;
  fn->functions_capacity = capacity;
  fn->functions = (HkFunction **) hk_reallocate(fn->functions,
    sizeof(*fn->functions) * capacity);
}

HkFunction *hk_function_new(int arity, HkString *name, HkString *file)
{
  HkFunction *fn = function_allocate(arity, name, file);
  hk_chunk_init(&fn->chunk);
  init_functions(fn);
  fn->num_nonlocals = 0;
  return fn;
}

void hk_function_free(HkFunction *fn)
{
  HkString *name = fn->name;
  if (name)
    hk_string_release(name);
  hk_string_release(fn->file);
  hk_chunk_deinit(&fn->chunk);
  free_functions(fn);
  free(fn);
}

void hk_function_release(HkFunction *fn)
{
  hk_decr_ref(fn);
  if (hk_is_unreachable(fn))
    hk_function_free(fn);
}

void hk_function_add_child(HkFunction *fn, HkFunction *child)
{
  grow_functions(fn);
  hk_incr_ref(child);
  fn->functions[fn->functions_length] = child;
  ++fn->functions_length;
}

void hk_function_serialize(HkFunction *fn, FILE *stream)
{
  fwrite(&fn->arity, sizeof(fn->arity), 1, stream);
  hk_string_serialize(fn->name, stream);
  hk_string_serialize(fn->file, stream);
  hk_chunk_serialize(&fn->chunk, stream);
  fwrite(&fn->functions_capacity, sizeof(fn->functions_capacity), 1, stream);
  fwrite(&fn->functions_length, sizeof(fn->functions_length), 1, stream);
  HkFunction **functions = fn->functions;
  for (int i = 0; i < fn->functions_length; ++i)
    hk_function_serialize(functions[i], stream);
  fwrite(&fn->num_nonlocals, sizeof(fn->num_nonlocals), 1, stream);
}

HkFunction *hk_function_deserialize(FILE *stream)
{
  int arity;
  if (fread(&arity, sizeof(arity), 1, stream) != 1)
    return NULL;
  HkString *name = hk_string_deserialize(stream);
  if (!name)
    return NULL;
  HkString *file = hk_string_deserialize(stream);
  if (!file)
    return NULL;
  HkFunction *fn = function_allocate(arity, name, file);
  if (!hk_chunk_deserialize(&fn->chunk, stream))
    return NULL;
  if (fread(&fn->functions_capacity, sizeof(fn->functions_capacity), 1, stream) != 1)
    return NULL;
  if (fread(&fn->functions_length, sizeof(fn->functions_length), 1, stream) != 1)
    return NULL;
  HkFunction **functions = (HkFunction **) hk_allocate(sizeof(*functions) * fn->functions_capacity);
  for (int i = 0; i < fn->functions_length; ++i)
  {
    HkFunction *fn = hk_function_deserialize(stream);
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

HkClosure *hk_closure_new(HkFunction *fn)
{
  int size = sizeof(HkClosure) + sizeof(HkValue) * (fn->num_nonlocals - 1);
  HkClosure *cl = (HkClosure *) hk_allocate(size);
  cl->ref_count = 0;
  hk_incr_ref(fn);
  cl->fn = fn;
  return cl;
}

void hk_closure_free(HkClosure *cl)
{
  HkFunction *fn = cl->fn;
  int num_nonlocals = fn->num_nonlocals;
  hk_function_release(fn);
  for (int i = 0; i < num_nonlocals; ++i)
    hk_value_release(cl->nonlocals[i]);
  free(cl);
}

void hk_closure_release(HkClosure *cl)
{
  hk_decr_ref(cl);
  if (hk_is_unreachable(cl))
    hk_closure_free(cl);
}

HkNative *hk_native_new(HkString *name, int arity, void (*call)(struct hk_state *, HkValue *))
{
  HkNative *native = (HkNative *) hk_allocate(sizeof(*native));
  native->ref_count = 0;
  native->arity = arity;
  hk_incr_ref(name);
  native->name = name;
  native->call = call;
  return native;
}

void hk_native_free(HkNative *native)
{
  hk_string_release(native->name);
  free(native);
}

void hk_native_release(HkNative *native)
{
  hk_decr_ref(native);
  if (hk_is_unreachable(native))
    hk_native_free(native);
}
