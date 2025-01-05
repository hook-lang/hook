//
// chunk.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_CHUNK_H
#define HK_CHUNK_H

#include "array.h"

typedef enum
{
  HK_OP_NIL,                    HK_OP_FALSE,                  HK_OP_TRUE,
  HK_OP_INT,                    HK_OP_CONSTANT,               HK_OP_RANGE,
  HK_OP_ARRAY,                  HK_OP_STRUCT,                 HK_OP_INSTANCE,
  HK_OP_CONSTRUCT,              HK_OP_ITERATOR,               HK_OP_CLOSURE,
  HK_OP_UNPACK_ARRAY,           HK_OP_UNPACK_STRUCT,          HK_OP_POP,
  HK_OP_GLOBAL,                 HK_OP_NONLOCAL,               HK_OP_GET_LOCAL,
  HK_OP_SET_LOCAL,              HK_OP_APPEND_ELEMENT,         HK_OP_GET_ELEMENT,
  HK_OP_FETCH_ELEMENT,          HK_OP_SET_ELEMENT,            HK_OP_PUT_ELEMENT,
  HK_OP_DELETE_ELEMENT,         HK_OP_INPLACE_APPEND_ELEMENT, HK_OP_INPLACE_PUT_ELEMENT,
  HK_OP_INPLACE_DELETE_ELEMENT, HK_OP_GET_FIELD,              HK_OP_FETCH_FIELD,
  HK_OP_SET_FIELD,              HK_OP_PUT_FIELD,              HK_OP_INPLACE_PUT_FIELD,
  HK_OP_CURRENT,                HK_OP_JUMP,                   HK_OP_JUMP_IF_FALSE,
  HK_OP_JUMP_IF_TRUE,           HK_OP_JUMP_IF_TRUE_OR_POP,    HK_OP_JUMP_IF_FALSE_OR_POP,
  HK_OP_JUMP_IF_NOT_EQUAL,      HK_OP_JUMP_IF_NOT_VALID,      HK_OP_NEXT,
  HK_OP_EQUAL,                  HK_OP_GREATER,                HK_OP_LESS,
  HK_OP_NOT_EQUAL,              HK_OP_NOT_GREATER,            HK_OP_NOT_LESS,
  HK_OP_BITWISE_OR,             HK_OP_BITWISE_XOR,            HK_OP_BITWISE_AND,
  HK_OP_LEFT_SHIFT,             HK_OP_RIGHT_SHIFT,            HK_OP_ADD,
  HK_OP_SUBTRACT,               HK_OP_MULTIPLY,               HK_OP_DIVIDE,
  HK_OP_QUOTIENT,               HK_OP_REMAINDER,              HK_OP_NEGATE,
  HK_OP_NOT,                    HK_OP_BITWISE_NOT,            HK_OP_INCREMENT,
  HK_OP_DECREMENT,              HK_OP_CALL,                   HK_OP_LOAD_MODULE,
  HK_OP_RETURN,                 HK_OP_RETURN_NIL
} HkOpcode;

typedef struct
{
  int no;
  int offset;
} HkLine;

typedef struct
{
  int     codeCapacity;
  int     codeLength;
  uint8_t *code;
  int     linesCapacity;
  int     linesLength;
  HkLine  *lines;
  HkArray *consts;
} HkChunk;

void hk_chunk_init(HkChunk *chunk);
void hk_chunk_deinit(HkChunk *chunk);
void hk_chunk_emit_byte(HkChunk *chunk, uint8_t byte);
void hk_chunk_emit_word(HkChunk *chunk, uint16_t word);
void hk_chunk_emit_opcode(HkChunk *chunk, HkOpcode op);
void hk_chunk_append_line(HkChunk *chunk, int no);
int hk_chunk_get_line(HkChunk *chunk, int offset);
void hk_chunk_serialize(HkChunk *chunk, FILE *stream);
bool hk_chunk_deserialize(HkChunk *chunk, FILE *stream);

#endif // HK_CHUNK_H
