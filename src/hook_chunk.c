//
// Hook Programming Language
// hook_chunk.c
//

#include "hook_chunk.h"
#include <stdlib.h>
#include "hook_utils.h"
#include "hook_memory.h"

#define CHUNK_MIN_CAPACITY (1 << 3)

static inline void ensure_capacity(hk_chunk_t *chunk, int min_capacity);

static inline void ensure_capacity(hk_chunk_t *chunk, int min_capacity)
{
  if (min_capacity <= chunk->capacity)
    return;
  int capacity = hk_power_of_two_ceil(min_capacity);
  chunk->capacity = capacity;
  chunk->bytes = (uint8_t *) hk_reallocate(chunk->bytes, capacity);
}

void hk_chunk_init(hk_chunk_t *chunk)
{
  chunk->capacity = CHUNK_MIN_CAPACITY;
  chunk->length = 0;
  chunk->bytes = (uint8_t *) hk_allocate(chunk->capacity);
}

void hk_chunk_free(hk_chunk_t *chunk)
{
  free(chunk->bytes);
}

void hk_chunk_emit_byte(hk_chunk_t *chunk, uint8_t byte)
{
  ensure_capacity(chunk, chunk->length + 1);
  chunk->bytes[chunk->length] = byte;
  ++chunk->length;
}

void hk_chunk_emit_word(hk_chunk_t *chunk, uint16_t word)
{
  ensure_capacity(chunk, chunk->length + 2);
  *((uint16_t *) &chunk->bytes[chunk->length]) = word;
  chunk->length += 2;
}

void hk_chunk_emit_opcode(hk_chunk_t *chunk, int op)
{
  hk_chunk_emit_byte(chunk, (uint8_t) op);
}

void hk_chunk_serialize(hk_chunk_t *chunk, FILE *stream)
{
  fwrite(&chunk->capacity, sizeof(chunk->capacity), 1, stream);
  fwrite(&chunk->length, sizeof(chunk->length), 1, stream);
  fwrite(chunk->bytes, chunk->length, 1, stream);
}

bool hk_chunk_deserialize(hk_chunk_t *chunk, FILE *stream)
{
  if (fread(&chunk->capacity, sizeof(chunk->capacity), 1, stream) != 1)
    return false;
  if (fread(&chunk->length, sizeof(chunk->length), 1, stream) != 1)
    return false;
  chunk->bytes = (uint8_t *) hk_allocate(chunk->capacity);
  return fread(chunk->bytes, chunk->length, 1, stream) == 1;
}
