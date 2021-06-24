//
// Hook Programming Language
// vm.c
//

#include "vm.h"
#include <stdio.h>
#include <math.h>

static inline int read_byte(uint8_t **pc);
static inline int read_word(uint8_t **pc);
static inline void add(stack_t *stk);
static inline void subtract(stack_t *stk);
static inline void multiply(stack_t *stk);
static inline void divide(stack_t *stk);
static inline void modulo(stack_t *stk);
static inline void negate(stack_t *stk);
static inline void print(stack_t *stk);

static inline void add(stack_t *stk)
{
  value_t val2 = stack_pop(stk);
  value_t val1 = stack_pop(stk);
  stack_push_double(stk, val1 + val2);
}

static inline void subtract(stack_t *stk)
{
  value_t val2 = stack_pop(stk);
  value_t val1 = stack_pop(stk);
  stack_push_double(stk, val1 - val2);
}

static inline void multiply(stack_t *stk)
{
  value_t val2 = stack_pop(stk);
  value_t val1 = stack_pop(stk);
  stack_push_double(stk, val1 * val2);
}

static inline void divide(stack_t *stk)
{
  value_t val2 = stack_pop(stk);
  value_t val1 = stack_pop(stk);
  stack_push_double(stk, val1 / val2);
}

static inline void modulo(stack_t *stk)
{
  value_t val2 = stack_pop(stk);
  value_t val1 = stack_pop(stk);
  stack_push_double(stk, fmod(val1, val2));
}

static inline void negate(stack_t *stk)
{
  value_t val = stack_pop(stk);
  stack_push_double(stk, -val);
}

static inline void print(stack_t *stk)
{
  value_t val = stack_pop(stk);
  printf("%g\n", val);
}

static inline int read_byte(uint8_t **pc)
{
  int byte = **pc;
  ++(*pc);
  return byte;
}

static inline int read_word(uint8_t **pc)
{
  int word = *((uint16_t *) *pc);
  *pc += 2;
  return word;
}

void execute(stack_t *stk, chunk_t *chunk)
{
  uint8_t *pc = chunk->bytes;
  for (;;)
  {
    opcode_t op = (opcode_t) read_byte(&pc);
    switch (op)
    {
    case OP_INT:
      stack_push_double(stk, read_word(&pc));
      break;
    case OP_ADD:
      add(stk);
      break;
    case OP_SUBTRACT:
      subtract(stk);
      break;
    case OP_MULTIPLY:
      multiply(stk);
      break;
    case OP_DIVIDE:
      divide(stk);
      break;
    case OP_MODULO:
      modulo(stk);
      break;
    case OP_NEGATE:
      negate(stk);
      break;
    case OP_PRINT:
      print(stk);
      break;
    case OP_RETURN:
      return;
    }
  }
}
