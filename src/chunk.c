//
// Hook Programming Language
// chunk.c
//

#include "chunk.h"
#include <stdlib.h>
#include "common.h"
#include "memory.h"

#define CHUNK_MIN_CAPACITY (1 << 3)

static inline void resize(chunk_t *chunk, int min_capacity);

static inline void resize(chunk_t *chunk, int min_capacity)
{
  if (min_capacity <= chunk->capacity)
    return;
  int capacity = nearest_power_of_two(chunk->capacity, min_capacity);
  chunk->capacity = capacity;
  chunk->bytes = (uint8_t *) reallocate(chunk->bytes, capacity);
}

void chunk_init(chunk_t *chunk)
{
  chunk->capacity = CHUNK_MIN_CAPACITY;
  chunk->length = 0;
  chunk->bytes = (uint8_t *) allocate(chunk->capacity);
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

void chunk_serialize(chunk_t *chunk, FILE *stream)
{
  fwrite(&chunk->capacity, sizeof(chunk->capacity), 1, stream);
  fwrite(&chunk->length, sizeof(chunk->length), 1, stream);
  fwrite(chunk->bytes, chunk->length, 1, stream);
}

bool chunk_deserialize(chunk_t *chunk, FILE *stream)
{
  if (fread(&chunk->capacity, sizeof(chunk->capacity), 1, stream) != 1)
    return false;
  if (fread(&chunk->length, sizeof(chunk->length), 1, stream) != 1)
    return false;
  chunk->bytes = (uint8_t *) allocate(chunk->capacity);
  return fread(chunk->bytes, chunk->length, 1, stream) == 1;
}
