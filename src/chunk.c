//
// Hook Programming Language
// chunk.c
//

#include "chunk.h"
#include <stdlib.h>
#include "memory.h"

static inline void resize(chunk_t *chunk, int min_capacity);

static inline void resize(chunk_t *chunk, int min_capacity)
{
  if (min_capacity <= chunk->capacity)
    return;
  int capacity = chunk->capacity;
  while (capacity < min_capacity)
    capacity <<= 1;
  chunk->capacity = capacity;
  chunk->bytes = (uint8_t *) reallocate(chunk->bytes, capacity);
}

void chunk_init(chunk_t *chunk, int min_capacity)
{
  int capacity = CHUNK_MIN_CAPACITY;
  while (capacity < min_capacity)
    capacity <<= 1;
  chunk->capacity = capacity;
  chunk->length = 0;
  chunk->bytes = (uint8_t *) allocate(capacity);
}

void chunk_free(chunk_t *chunk)
{
  free(chunk->bytes);
}

void chunk_emit_byte(chunk_t *chunk, uint8_t byte)
{
  resize(chunk, chunk->length + 1);
  chunk->bytes[chunk->length] = byte;
  ++chunk->length;
}

void chunk_emit_word(chunk_t *chunk, uint16_t word)
{
  resize(chunk, chunk->length + 2);
  *((uint16_t *) &chunk->bytes[chunk->length]) = word;
  chunk->length += 2;
}

void chunk_emit_opcode(chunk_t *chunk, opcode_t op)
{
  chunk_emit_byte(chunk, (uint8_t) op);
}
