//
// Hook Programming Language
// chunk.h
//

#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum
{
  OP_NIL,
  OP_FALSE,
  OP_TRUE,
  OP_INT,
  OP_CONSTANT,
  OP_RANGE,
  OP_ARRAY,
  OP_STRUCT,
  OP_INSTANCE,
  OP_CONSTRUCT,
  OP_CLOSURE,
  OP_UNPACK,
  OP_DESTRUCT,
  OP_POP,
  OP_GLOBAL,
  OP_NONLOCAL,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_ADD_ELEMENT,
  OP_GET_ELEMENT,
  OP_FETCH_ELEMENT,
  OP_SET_ELEMENT,
  OP_PUT_ELEMENT,
  OP_DELETE_ELEMENT,
  OP_INPLACE_ADD_ELEMENT,
  OP_INPLACE_PUT_ELEMENT,
  OP_INPLACE_DELETE_ELEMENT,
  OP_GET_FIELD,
  OP_FETCH_FIELD,
  OP_SET_FIELD,
  OP_PUT_FIELD,
  OP_INPLACE_PUT_FIELD,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_OR,
  OP_AND,
  OP_MATCH,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_NOT_EQUAL,
  OP_NOT_GREATER,
  OP_NOT_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_MODULO,
  OP_NEGATE,
  OP_NOT,
  OP_INCR,
  OP_DECR,
  OP_CALL,
  OP_LOAD_MODULE,
  OP_RETURN,
  OP_RETURN_NIL
} opcode_t;

typedef struct
{
  int capacity;
  int length;
  uint8_t *bytes;
} chunk_t;

void chunk_init(chunk_t *chunk);
void chunk_free(chunk_t *chunk);
void chunk_emit_byte(chunk_t *chunk, uint8_t byte);
void chunk_emit_word(chunk_t *chunk, uint16_t word);
void chunk_emit_opcode(chunk_t *chunk, opcode_t op);
void chunk_serialize(chunk_t *chunk, FILE *stream);
bool chunk_deserialize(chunk_t *chunk, FILE *stream);

#endif // CHUNK_H
