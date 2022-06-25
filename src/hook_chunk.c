//
// Hook Programming Language
// hook_chunk.c
//

#include "hook_chunk.h"
#include <stdlib.h>
#include "hook_utils.h"
#include "hook_memory.h"

#define MIN_CAPACITY (1 << 3)

static inline void ensure_code_capacity(hk_chunk_t *chunk, int32_t min_capacity);
static inline void init_lines(hk_chunk_t *chunk);
static inline void grow_lines(hk_chunk_t *chunk);

static inline void ensure_code_capacity(hk_chunk_t *chunk, int32_t min_capacity)
{
  if (min_capacity <= chunk->code_capacity)
    return;
  int32_t capacity = hk_power_of_two_ceil(min_capacity);
  chunk->code_capacity = capacity;
  chunk->code = (uint8_t *) hk_reallocate(chunk->code, capacity);
}

static inline void init_lines(hk_chunk_t *chunk)
{
  chunk->lines_capacity = MIN_CAPACITY;
  chunk->lines_length = 0;
  chunk->lines = (hk_line_t *) hk_allocate(sizeof(*chunk->lines) * chunk->lines_capacity);
}

static inline void grow_lines(hk_chunk_t *chunk)
{
  if (chunk->lines_length < chunk->lines_capacity)
    return;
  int32_t capacity = chunk->lines_capacity << 1;
  chunk->lines_capacity = capacity;
  chunk->lines = (hk_line_t *) hk_reallocate(chunk->lines,
    sizeof(*chunk->lines) * capacity);
}

void hk_chunk_init(hk_chunk_t *chunk)
{
  chunk->code_capacity = MIN_CAPACITY;
  chunk->code_length = 0;
  chunk->code = (uint8_t *) hk_allocate(chunk->code_capacity);
  init_lines(chunk);
  chunk->consts = hk_array_new();
}

void hk_chunk_free(hk_chunk_t *chunk)
{
  free(chunk->code);
  free(chunk->lines);
  hk_array_free(chunk->consts);
}

void hk_chunk_emit_byte(hk_chunk_t *chunk, uint8_t byte)
{
  ensure_code_capacity(chunk, chunk->code_length + 1);
  chunk->code[chunk->code_length] = byte;
  ++chunk->code_length;
}

void hk_chunk_emit_word(hk_chunk_t *chunk, uint16_t word)
{
  ensure_code_capacity(chunk, chunk->code_length + 2);
  *((uint16_t *) &chunk->code[chunk->code_length]) = word;
  chunk->code_length += 2;
}

void hk_chunk_emit_opcode(hk_chunk_t *chunk, int32_t op)
{
  hk_chunk_emit_byte(chunk, (uint8_t) op);
}

void hk_chunk_add_line(hk_chunk_t *chunk, int32_t line_no)
{
  grow_lines(chunk);
  hk_line_t *line = &chunk->lines[chunk->lines_length];
  line->no = line_no;
  line->offset = chunk->code_length;
  ++chunk->lines_length;
}

int32_t hk_chunk_get_line(hk_chunk_t *chunk, int32_t offset)
{
  int32_t line_no = -1;
  hk_line_t *lines = chunk->lines;
  for (int32_t i = 0; i < chunk->lines_length; ++i)
  {
    hk_line_t *line = &lines[i];
    if (line->offset != offset)
      continue;
    line_no = line->no;
    break;
  }
  hk_assert(line_no != -1, "chunk must contain the line number");
  return line_no;
}

void hk_chunk_serialize(hk_chunk_t *chunk, FILE *stream)
{
  fwrite(&chunk->code_capacity, sizeof(chunk->code_capacity), 1, stream);
  fwrite(&chunk->code_length, sizeof(chunk->code_length), 1, stream);
  fwrite(chunk->code, chunk->code_length, 1, stream);
  fwrite(&chunk->lines_capacity, sizeof(chunk->lines_capacity), 1, stream);
  fwrite(&chunk->lines_length, sizeof(chunk->lines_length), 1, stream);
  for (int32_t i = 0; i < chunk->lines_length; ++i)
  {
    hk_line_t *line = &chunk->lines[i];
    fwrite(line, sizeof(*line), 1, stream);
  }
  hk_array_serialize(chunk->consts, stream);
}

bool hk_chunk_deserialize(hk_chunk_t *chunk, FILE *stream)
{
  if (fread(&chunk->code_capacity, sizeof(chunk->code_capacity), 1, stream) != 1)
    return false;
  if (fread(&chunk->code_length, sizeof(chunk->code_length), 1, stream) != 1)
    return false;
  chunk->code = (uint8_t *) hk_allocate(chunk->code_capacity);
  if (fread(chunk->code, chunk->code_length, 1, stream) != 1)
    return false;
  if (fread(&chunk->lines_capacity, sizeof(chunk->lines_capacity), 1, stream) != 1)
    return false;
  if (fread(&chunk->lines_length, sizeof(chunk->lines_length), 1, stream) != 1)
    return false;
  chunk->lines = (hk_line_t *) hk_allocate(sizeof(*chunk->lines) * chunk->lines_capacity);
  for (int32_t i = 0; i < chunk->lines_length; ++i)
  {
    hk_line_t *line = &chunk->lines[i];
    if (fread(line, sizeof(*line), 1, stream) != 1)
      return false;
  }
  chunk->consts = hk_array_deserialize(stream);
  if (!chunk->consts)
    return false;
  hk_incr_ref(chunk->consts);
  return true;
}
