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
  fn->refCount = 0;
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
  fn->functionsCapacity = MIN_CAPACITY;
  fn->functionsLength = 0;
  fn->functions = (HkFunction **) hk_allocate(sizeof(*fn->functions) * fn->functionsCapacity);
}

static inline void free_functions(HkFunction *fn)
{
  for (int i = 0; i < fn->functionsLength; ++i)
    hk_function_release(fn->functions[i]);
  free(fn->functions);
}

static inline void grow_functions(HkFunction *fn)
{
  if (fn->functionsLength < fn->functionsCapacity)
    return;
  uint8_t capacity = fn->functionsCapacity << 1;
  fn->functionsCapacity = capacity;
  fn->functions = (HkFunction **) hk_reallocate(fn->functions,
    sizeof(*fn->functions) * capacity);
}

HkFunction *hk_function_new(int arity, HkString *name, HkString *file)
{
  HkFunction *fn = function_allocate(arity, name, file);
  hk_chunk_init(&fn->chunk);
  init_functions(fn);
  fn->numNonlocals = 0;
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
  fn->functions[fn->functionsLength] = child;
  ++fn->functionsLength;
}

void hk_function_serialize(HkFunction *fn, FILE *stream)
{
  fwrite(&fn->arity, sizeof(fn->arity), 1, stream);
  hk_string_serialize(fn->name, stream);
  hk_string_serialize(fn->file, stream);
  hk_chunk_serialize(&fn->chunk, stream);
  fwrite(&fn->functionsCapacity, sizeof(fn->functionsCapacity), 1, stream);
  fwrite(&fn->functionsLength, sizeof(fn->functionsLength), 1, stream);
  HkFunction **functions = fn->functions;
  for (int i = 0; i < fn->functionsLength; ++i)
    hk_function_serialize(functions[i], stream);
  fwrite(&fn->numNonlocals, sizeof(fn->numNonlocals), 1, stream);
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
  if (fread(&fn->functionsCapacity, sizeof(fn->functionsCapacity), 1, stream) != 1)
    return NULL;
  if (fread(&fn->functionsLength, sizeof(fn->functionsLength), 1, stream) != 1)
    return NULL;
  HkFunction **functions = (HkFunction **) hk_allocate(sizeof(*functions) * fn->functionsCapacity);
  for (int i = 0; i < fn->functionsLength; ++i)
  {
    HkFunction *nestedFn = hk_function_deserialize(stream);
    if (!nestedFn) return NULL;
    hk_incr_ref(nestedFn);
    functions[i] = nestedFn;
  }
  fn->functions = functions;
  if (fread(&fn->numNonlocals, sizeof(fn->numNonlocals), 1, stream) != 1)
    return NULL;
  return fn;
}

HkClosure *hk_closure_new(HkFunction *fn)
{
  int size = sizeof(HkClosure) + sizeof(HkValue) * (fn->numNonlocals - 1);
  HkClosure *cl = (HkClosure *) hk_allocate(size);
  cl->refCount = 0;
  hk_incr_ref(fn);
  cl->fn = fn;
  return cl;
}

void hk_closure_free(HkClosure *cl)
{
  HkFunction *fn = cl->fn;
  int numNonlocals = fn->numNonlocals;
  hk_function_release(fn);
  for (int i = 0; i < numNonlocals; ++i)
    hk_value_release(cl->nonlocals[i]);
  free(cl);
}

void hk_closure_release(HkClosure *cl)
{
  hk_decr_ref(cl);
  if (hk_is_unreachable(cl))
    hk_closure_free(cl);
}

HkNative *hk_native_new(HkString *name, int arity, HkCallFn call)
{
  HkNative *native = (HkNative *) hk_allocate(sizeof(*native));
  native->refCount = 0;
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
