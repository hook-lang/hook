//
// The Hook Programming Language
// chunk.c
//

#include <hook/chunk.h>
#include <stdlib.h>
#include <hook/memory.h>
#include <hook/utils.h>

#define MIN_CAPACITY (1 << 3)

static inline void ensure_code_capacity(HkChunk *chunk, int minCapacity);
static inline void init_lines(HkChunk *chunk);
static inline void grow_lines(HkChunk *chunk);

static inline void ensure_code_capacity(HkChunk *chunk, int minCapacity)
{
  if (minCapacity <= chunk->code_capacity)
    return;
  int capacity = hk_power_of_two_ceil(minCapacity);
  chunk->code_capacity = capacity;
  chunk->code = (uint8_t *) hk_reallocate(chunk->code, capacity);
}

static inline void init_lines(HkChunk *chunk)
{
  chunk->lines_capacity = MIN_CAPACITY;
  chunk->lines_length = 0;
  chunk->lines = (HkLine *) hk_allocate(sizeof(*chunk->lines) * chunk->lines_capacity);
}

static inline void grow_lines(HkChunk *chunk)
{
  if (chunk->lines_length < chunk->lines_capacity)
    return;
  int capacity = chunk->lines_capacity << 1;
  chunk->lines_capacity = capacity;
  chunk->lines = (HkLine *) hk_reallocate(chunk->lines,
    sizeof(*chunk->lines) * capacity);
}

void hk_chunk_init(HkChunk *chunk)
{
  chunk->code_capacity = MIN_CAPACITY;
  chunk->code_length = 0;
  chunk->code = (uint8_t *) hk_allocate(chunk->code_capacity);
  init_lines(chunk);
  chunk->consts = hk_array_new();
}

void hk_chunk_deinit(HkChunk *chunk)
{
  free(chunk->code);
  free(chunk->lines);
  hk_array_free(chunk->consts);
}

void hk_chunk_emit_byte(HkChunk *chunk, uint8_t byte)
{
  ensure_code_capacity(chunk, chunk->code_length + 1);
  chunk->code[chunk->code_length] = byte;
  ++chunk->code_length;
}

void hk_chunk_emit_word(HkChunk *chunk, uint16_t word)
{
  ensure_code_capacity(chunk, chunk->code_length + 2);
  *((uint16_t *) &chunk->code[chunk->code_length]) = word;
  chunk->code_length += 2;
}

void hk_chunk_emit_opcode(HkChunk *chunk, HkOpCode op)
{
  hk_chunk_emit_byte(chunk, (uint8_t) op);
}

void hk_chunk_add_line(HkChunk *chunk, int line_no)
{
  grow_lines(chunk);
  HkLine *line = &chunk->lines[chunk->lines_length];
  line->no = line_no;
  line->offset = chunk->code_length;
  ++chunk->lines_length;
}

int hk_chunk_get_line(HkChunk *chunk, int offset)
{
  int result = 1;
  HkLine *lines = chunk->lines;
  for (int i = 0; i < chunk->lines_length; ++i)
  {
    HkLine *line = &lines[i];
    if (line->offset >= offset)
      break;
    result = line->no;
  }
  return result;
}

void hk_chunk_serialize(HkChunk *chunk, FILE *stream)
{
  fwrite(&chunk->code_capacity, sizeof(chunk->code_capacity), 1, stream);
  fwrite(&chunk->code_length, sizeof(chunk->code_length), 1, stream);
  fwrite(chunk->code, chunk->code_length, 1, stream);
  fwrite(&chunk->lines_capacity, sizeof(chunk->lines_capacity), 1, stream);
  fwrite(&chunk->lines_length, sizeof(chunk->lines_length), 1, stream);
  for (int i = 0; i < chunk->lines_length; ++i)
  {
    HkLine *line = &chunk->lines[i];
    fwrite(line, sizeof(*line), 1, stream);
  }
  hk_array_serialize(chunk->consts, stream);
}

bool hk_chunk_deserialize(HkChunk *chunk, FILE *stream)
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
  chunk->lines = (HkLine *) hk_allocate(sizeof(*chunk->lines) * chunk->lines_capacity);
  for (int i = 0; i < chunk->lines_length; ++i)
  {
    HkLine *line = &chunk->lines[i];
    if (fread(line, sizeof(*line), 1, stream) != 1)
      return false;
  }
  chunk->consts = hk_array_deserialize(stream);
  if (!chunk->consts)
    return false;
  hk_incr_ref(chunk->consts);
  return true;
}
