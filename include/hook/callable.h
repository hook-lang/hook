//
// callable.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_CALLABLE_H
#define HK_CALLABLE_H

#include "chunk.h"
#include "string.h"

typedef struct HkFunction
{
  HK_OBJECT_HEADER
  int               arity;
  HkString          *name;
  HkString          *file;
  HkChunk           chunk;
  uint8_t           functionsCapacity;
  uint8_t           functionsLength;
  struct HkFunction **functions;
  uint8_t           numNonlocals;
} HkFunction;

typedef struct
{
  HK_OBJECT_HEADER
  HkFunction *fn;
  HkValue    nonlocals[1];
} HkClosure;

struct HkVM;

typedef void (*HkCallFn)(struct HkVM *, HkValue *);

typedef struct
{
  HK_OBJECT_HEADER
  int      arity;
  HkString *name;
  HkCallFn call;
} HkNative;

HkFunction *hk_function_new(int arity, HkString *name, HkString *file);
void hk_function_free(HkFunction *fn);
void hk_function_release(HkFunction *fn);
void hk_function_append_child(HkFunction *fn, HkFunction *child);
void hk_function_serialize(HkFunction *fn, FILE *stream);
HkFunction *hk_function_deserialize(FILE *stream);
HkClosure *hk_closure_new(HkFunction *fn);
void hk_closure_free(HkClosure *cl);
void hk_closure_release(HkClosure *cl);
HkNative *hk_native_new(HkString *name, int arity, HkCallFn call);
void hk_native_free(HkNative *native);
void hk_native_release(HkNative *native);

#endif // HK_CALLABLE_H
