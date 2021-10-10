//
// Hook Programming Language
// chunk.h
//

#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

typedef enum
{
  OP_NULL,
  OP_FALSE,
  OP_TRUE,
  OP_INT,
  OP_CONSTANT,
  OP_ARRAY,
  OP_UNPACK,
  OP_POP,
  OP_GLOBAL,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_APPEND,
  OP_GET_ELEMENT,
  OP_FETCH_ELEMENT,
  OP_SET_ELEMENT,
  OP_PUT_ELEMENT,
  OP_DELETE,
  OP_INPLACE_APPEND,
  OP_INPLACE_PUT_ELEMENT,
  OP_INPLACE_DELETE,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_JUMP_IF_TRUE,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_MODULO,
  OP_NEGATE,
  OP_NOT,
  OP_CALL,
  OP_RETURN
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

#endif
