//
// Hook Programming Language
// hook_chunk.h
//

#ifndef HOOK_CHUNK_H
#define HOOK_CHUNK_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define HK_OP_NIL                    0x00
#define HK_OP_FALSE                  0x01
#define HK_OP_TRUE                   0x02
#define HK_OP_INT                    0x03
#define HK_OP_CONSTANT               0x04
#define HK_OP_RANGE                  0x05
#define HK_OP_ARRAY                  0x06
#define HK_OP_STRUCT                 0x07
#define HK_OP_INSTANCE               0x08
#define HK_OP_CONSTRUCT              0x09
#define HK_OP_CLOSURE                0x0a
#define HK_OP_UNPACK                 0x0b
#define HK_OP_DESTRUCT               0x0c
#define HK_OP_POP                    0x0d
#define HK_OP_GLOBAL                 0x0e
#define HK_OP_NONLOCAL               0x0f
#define HK_OP_GET_LOCAL              0x10
#define HK_OP_SET_LOCAL              0x11
#define HK_OP_ADD_ELEMENT            0x12
#define HK_OP_GET_ELEMENT            0x13
#define HK_OP_FETCH_ELEMENT          0x14
#define HK_OP_SET_ELEMENT            0x15
#define HK_OP_PUT_ELEMENT            0x16
#define HK_OP_DELETE_ELEMENT         0x17
#define HK_OP_INPLACE_ADD_ELEMENT    0x18
#define HK_OP_INPLACE_PUT_ELEMENT    0x19
#define HK_OP_INPLACE_DELETE_ELEMENT 0x1a
#define HK_OP_GET_FIELD              0x1b
#define HK_OP_FETCH_FIELD            0x1c
#define HK_OP_SET_FIELD              0x1d
#define HK_OP_PUT_FIELD              0x1e
#define HK_OP_INPLACE_PUT_FIELD      0x1f
#define HK_OP_JUMP                   0x20
#define HK_OP_JUMP_IF_FALSE          0x21
#define HK_OP_OR                     0x22
#define HK_OP_AND                    0x23
#define HK_OP_MATCH                  0x24
#define HK_OP_EQUAL                  0x25
#define HK_OP_GREATER                0x26
#define HK_OP_LESS                   0x27
#define HK_OP_NOT_EQUAL              0x28
#define HK_OP_NOT_GREATER            0x29
#define HK_OP_NOT_LESS               0x2a
#define HK_OP_ADD                    0x2b
#define HK_OP_SUBTRACT               0x2c
#define HK_OP_MULTIPLY               0x2d
#define HK_OP_DIVIDE                 0x2e
#define HK_OP_QUOTIENT               0x2f
#define HK_OP_REMINDER               0x30
#define HK_OP_NEGATE                 0x31
#define HK_OP_NOT                    0x32
#define HK_OP_INCR                   0x33
#define HK_OP_DECR                   0x34
#define HK_OP_CALL                   0x35
#define HK_OP_LOAD_MODULE            0x36
#define HK_OP_RETURN                 0x37
#define HK_OP_RETURN_NIL             0X38

typedef struct
{
  int32_t capacity;
  int32_t length;
  uint8_t *bytes;
} hk_chunk_t;

void hk_chunk_init(hk_chunk_t *chunk);
void hk_chunk_free(hk_chunk_t *chunk);
void hk_chunk_emit_byte(hk_chunk_t *chunk, uint8_t byte);
void hk_chunk_emit_word(hk_chunk_t *chunk, uint16_t word);
void hk_chunk_emit_opcode(hk_chunk_t *chunk, int32_t op);
void hk_chunk_serialize(hk_chunk_t *chunk, FILE *stream);
bool hk_chunk_deserialize(hk_chunk_t *chunk, FILE *stream);

#endif // HOOK_CHUNK_H
