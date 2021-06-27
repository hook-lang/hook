//
// Hook Programming Language
// chunk.h
//

#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

#define CHUNK_MIN_CAPACITY 8

typedef enum
{
  OP_NULL,
  OP_FALSE,
  OP_TRUE,
  OP_INT,
  OP_CONSTANT,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_MODULO,
  OP_NEGATE,
  OP_PRINT,
  OP_RETURN
} opcode_t;

typedef struct
{
  int capacity;
  int length;
  uint8_t *bytes;
} chunk_t;

void chunk_init(chunk_t *chunk, int min_capacity);
void chunk_free(chunk_t *chunk);
void chunk_write_byte(chunk_t *chunk, uint8_t byte);
void chunk_write_word(chunk_t *chunk, uint16_t word);
void chunk_emit_opcode(chunk_t *chunk, opcode_t op);

#endif
