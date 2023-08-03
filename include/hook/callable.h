//
// The Hook Programming Language
// callable.h
//

#ifndef HK_CALLABLE_H
#define HK_CALLABLE_H

#include <hook/string.h>
#include <hook/chunk.h>

typedef struct hk_function
{
  HK_OBJECT_HEADER
  int arity;
  HkString *name;
  HkString *file;
  HkChunk chunk;
  uint8_t functions_capacity;
  uint8_t functions_length;
  struct hk_function **functions;
  uint8_t num_nonlocals;
} HkFunction;

typedef struct
{
  HK_OBJECT_HEADER
  HkFunction *fn;
  HkValue nonlocals[1];
} HkClosure;

struct hk_state;

typedef struct
{
  HK_OBJECT_HEADER
  int arity;
  HkString *name;
  void (*call)(struct hk_state *, HkValue *);
} HkNative;

HkFunction *hk_function_new(int arity, HkString *name, HkString *file);
void hk_function_free(HkFunction *fn);
void hk_function_release(HkFunction *fn);
void hk_function_add_child(HkFunction *fn, HkFunction *child);
void hk_function_serialize(HkFunction *fn, FILE *stream);
HkFunction *hk_function_deserialize(FILE *stream);
HkClosure *hk_closure_new(HkFunction *fn);
void hk_closure_free(HkClosure *cl);
void hk_closure_release(HkClosure *cl);
HkNative *hk_native_new(HkString *name, int arity, void (*call)(struct hk_state *, HkValue *));
void hk_native_free(HkNative *native);
void hk_native_release(HkNative *native);

#endif // HK_CALLABLE_H
