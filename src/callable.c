//
// Hook Programming Language
// callable.c
//

#include "callable.h"
#include <stdlib.h>
#include "common.h"
#include "memory.h"

#define MIN_CAPACITY 8

static inline void init_lines(prototype_t *proto);
static inline void init_protos(prototype_t *proto);
static inline void free_protos(prototype_t *proto);
static inline void resize_lines(prototype_t *proto);
static inline void resize_protos(prototype_t *proto);

static inline void init_lines(prototype_t *proto)
{
  proto->lines_capacity = MIN_CAPACITY;
  proto->num_lines = 0;
  proto->lines = (line_t *) allocate(sizeof(*proto->lines) * proto->lines_capacity);
}

static inline void init_protos(prototype_t *proto)
{
  proto->protos_capacity = MIN_CAPACITY;
  proto->num_protos = 0;
  proto->protos = (prototype_t **) allocate(sizeof(*proto->protos) * proto->protos_capacity);
}

static inline void free_protos(prototype_t *proto)
{
  for (int i = 0; i < proto->num_protos; ++i)
  {
    prototype_t *child = proto->protos[i];
    DECR_REF(child);
    if (IS_UNREACHABLE(child))
      prototype_free(child);
  }
  free(proto->protos);
}

static inline void resize_lines(prototype_t *proto)
{
  if (proto->num_lines < proto->lines_capacity)
    return;
  int capacity = proto->lines_capacity << 1;
  proto->lines_capacity = capacity;
  proto->lines = (line_t *) reallocate(proto->lines,
    sizeof(*proto->lines) * capacity);
}

static inline void resize_protos(prototype_t *proto)
{
  if (proto->num_protos < proto->protos_capacity)
    return;
  uint8_t capacity = proto->protos_capacity << 1;
  proto->protos_capacity = capacity;
  proto->protos = (prototype_t **) reallocate(proto->protos,
    sizeof(*proto->protos) * capacity);
}

prototype_t *prototype_new(int arity, string_t *name, string_t *file)
{
  prototype_t *proto = (prototype_t *) allocate(sizeof(*proto));
  proto->ref_count = 0;
  proto->arity = arity;
  if (name)
    INCR_REF(name);
  proto->name = name;
  INCR_REF(file);
  proto->file = file;
  init_lines(proto);
  chunk_init(&proto->chunk);
  proto->consts = array_allocate(0);
  proto->consts->length = 0;
  init_protos(proto);
  proto->num_nonlocals = 0;
  return proto;
}

void prototype_free(prototype_t *proto)
{
  string_t *name = proto->name;
  if (name)
  {
    DECR_REF(name);
    if (IS_UNREACHABLE(name))
      string_free(name);
  }
  string_t *file = proto->file;
  DECR_REF(file);
  if (IS_UNREACHABLE(file))
    string_free(file);
  free(proto->lines);
  chunk_free(&proto->chunk);
  array_free(proto->consts);
  free_protos(proto);
  free(proto);
}

void prototype_add_line(prototype_t *proto, int line_no)
{
  resize_lines(proto);
  line_t *line = &proto->lines[proto->num_lines];
  line->no = line_no;
  line->offset = proto->chunk.length;
  ++proto->num_lines;
}

int prototype_get_line(prototype_t *proto, int offset)
{
  int line_no = -1;
  line_t *lines = proto->lines;
  for (int i = 0; i < proto->num_lines; ++i)
  {
    line_t *line = &lines[i];
    if (line->offset == offset)
    {
      line_no = line->no;
      break;
    }
  }
  ASSERT(line_no != -1, "prototype must contain the line number");
  return line_no;
}

void prototype_add_child(prototype_t *proto, prototype_t *child)
{
  resize_protos(proto);
  INCR_REF(child);
  proto->protos[proto->num_protos] = child;
  ++proto->num_protos;
}

function_t *function_new(prototype_t *proto)
{
  int size = sizeof(function_t) + sizeof(value_t) * proto->num_nonlocals;
  function_t *fn = (function_t *) allocate(size);
  fn->ref_count = 0;
  INCR_REF(proto);
  fn->proto = proto;
  return fn;
}

void function_free(function_t *fn)
{
  prototype_t *proto = fn->proto;
  int num_nonlocals = proto->num_nonlocals;
  DECR_REF(proto);
  if (IS_UNREACHABLE(proto))
    prototype_free(proto);
  for (int i = 0; i < num_nonlocals; ++i)
    value_release(fn->nonlocals[i]);
  free(fn);
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
