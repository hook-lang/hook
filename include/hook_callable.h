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
} hk_line_t;

typedef struct hk_function
{
  HK_OBJECT_HEADER
  int arity;
  hk_string_t *name;
  hk_string_t *file;
  int lines_capacity;
  int num_lines;
  hk_line_t *lines;
  hk_chunk_t chunk;
  hk_array_t *consts;
  uint8_t functions_capacity;
  uint8_t num_functions;
  struct hk_function **functions;
  uint8_t num_nonlocals;
} hk_function_t;

typedef struct
{
  HK_OBJECT_HEADER
  hk_function_t *fn;
  hk_value_t nonlocals[0];
} hk_closure_t;

struct hk_vm;

typedef struct
{
  HK_OBJECT_HEADER
  int arity;
  hk_string_t *name;
  int (*call)(struct hk_vm *, hk_value_t *);
} hk_native_t;

hk_function_t *hk_function_new(int arity, hk_string_t *name, hk_string_t *file);
void hk_function_free(hk_function_t *fn);
void hk_function_release(hk_function_t *fn);
void hk_function_add_line(hk_function_t *fn, int line_no);
int hk_function_get_line(hk_function_t *fn, int offset);
void hk_function_add_child(hk_function_t *fn, hk_function_t *child);
void hk_function_serialize(hk_function_t *fn, FILE *stream);
hk_function_t *hk_function_deserialize(FILE *stream);
hk_closure_t *hk_closure_new(hk_function_t *fn);
void hk_closure_free(hk_closure_t *cl);
void hk_closure_release(hk_closure_t *cl);
hk_native_t *hk_native_new(hk_string_t *name, int arity, int (*call)(struct hk_vm *, hk_value_t *));
void hk_native_free(hk_native_t *native);
void hk_native_release(hk_native_t *native);

#endif // HOOK_CALLABLE_H
