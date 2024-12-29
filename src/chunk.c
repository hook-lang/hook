//
// chunk.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
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
  if (minCapacity <= chunk->codeCapacity)
    return;
  int capacity = hk_power_of_two_ceil(minCapacity);
  chunk->codeCapacity = capacity;
  chunk->code = (uint8_t *) hk_reallocate(chunk->code, capacity);
}

static inline void init_lines(HkChunk *chunk)
{
  chunk->linesCapacity = MIN_CAPACITY;
  chunk->linesLength = 0;
  chunk->lines = (HkLine *) hk_allocate(sizeof(*chunk->lines) * chunk->linesCapacity);
}

static inline void grow_lines(HkChunk *chunk)
{
  if (chunk->linesLength < chunk->linesCapacity)
    return;
  int capacity = chunk->linesCapacity << 1;
  chunk->linesCapacity = capacity;
  chunk->lines = (HkLine *) hk_reallocate(chunk->lines,
    sizeof(*chunk->lines) * capacity);
}

void hk_chunk_init(HkChunk *chunk)
{
  chunk->codeCapacity = MIN_CAPACITY;
  chunk->codeLength = 0;
  chunk->code = (uint8_t *) hk_allocate(chunk->codeCapacity);
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
  ensure_code_capacity(chunk, chunk->codeLength + 1);
  chunk->code[chunk->codeLength] = byte;
  ++chunk->codeLength;
}

void hk_chunk_emit_word(HkChunk *chunk, uint16_t word)
{
  ensure_code_capacity(chunk, chunk->codeLength + 2);
  *((uint16_t *) &chunk->code[chunk->codeLength]) = word;
  chunk->codeLength += 2;
}

void hk_chunk_emit_opcode(HkChunk *chunk, HkOpCode op)
{
  hk_chunk_emit_byte(chunk, (uint8_t) op);
}

void hk_chunk_add_line(HkChunk *chunk, int no)
{
  grow_lines(chunk);
  HkLine *line = &chunk->lines[chunk->linesLength];
  line->no = no;
  line->offset = chunk->codeLength;
  ++chunk->linesLength;
}

int hk_chunk_get_line(HkChunk *chunk, int offset)
{
  int result = 1;
  HkLine *lines = chunk->lines;
  for (int i = 0; i < chunk->linesLength; ++i)
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
  fwrite(&chunk->codeCapacity, sizeof(chunk->codeCapacity), 1, stream);
  fwrite(&chunk->codeLength, sizeof(chunk->codeLength), 1, stream);
  fwrite(chunk->code, chunk->codeLength, 1, stream);
  fwrite(&chunk->linesCapacity, sizeof(chunk->linesCapacity), 1, stream);
  fwrite(&chunk->linesLength, sizeof(chunk->linesLength), 1, stream);
  for (int i = 0; i < chunk->linesLength; ++i)
  {
    HkLine *line = &chunk->lines[i];
    fwrite(line, sizeof(*line), 1, stream);
  }
  hk_array_serialize(chunk->consts, stream);
}

bool hk_chunk_deserialize(HkChunk *chunk, FILE *stream)
{
  if (fread(&chunk->codeCapacity, sizeof(chunk->codeCapacity), 1, stream) != 1)
    return false;
  if (fread(&chunk->codeLength, sizeof(chunk->codeLength), 1, stream) != 1)
    return false;
  chunk->code = (uint8_t *) hk_allocate(chunk->codeCapacity);
  if (fread(chunk->code, chunk->codeLength, 1, stream) != 1)
    return false;
  if (fread(&chunk->linesCapacity, sizeof(chunk->linesCapacity), 1, stream) != 1)
    return false;
  if (fread(&chunk->linesLength, sizeof(chunk->linesLength), 1, stream) != 1)
    return false;
  chunk->lines = (HkLine *) hk_allocate(sizeof(*chunk->lines) * chunk->linesCapacity);
  for (int i = 0; i < chunk->linesLength; ++i)
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
