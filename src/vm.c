//
// Hook Programming Language
// vm.c
//

#include "vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "common.h"
#include "memory.h"
#include "error.h"

static inline void push(vm_t *vm, value_t val);
static inline int read_byte(uint8_t **pc);
static inline int read_word(uint8_t **pc);
static inline void add(vm_t *vm);
static inline void subtract(vm_t *vm);
static inline void multiply(vm_t *vm);
static inline void divide(vm_t *vm);
static inline void modulo(vm_t *vm);
static inline void negate(vm_t *vm);
static inline void print(vm_t *vm);

static inline void push(vm_t *vm, value_t val)
{
  if (vm->index == vm->end)
    fatal_error("stack overflow");
  ++vm->index;
  vm->slots[vm->index] = val;
}

static inline void add(vm_t *vm)
{
  value_t val2 = vm_pop(vm);
  value_t val1 = VM_GET_TOP(vm);
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot add %s to %s", type_name(val2.type), type_name(val1.type));
  double data = val1.as_number + val2.as_number;
  VM_SET_TOP(vm, NUMBER_VALUE(data));
}

static inline void subtract(vm_t *vm)
{
  value_t val2 = vm_pop(vm);
  value_t val1 = VM_GET_TOP(vm);
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot subtract %s from %s", type_name(val2.type), type_name(val1.type));
  double data = val1.as_number - val2.as_number;
  VM_SET_TOP(vm, NUMBER_VALUE(data));
}

static inline void multiply(vm_t *vm)
{
  value_t val2 = vm_pop(vm);
  value_t val1 = VM_GET_TOP(vm);
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot multiply %s to %s", type_name(val2.type), type_name(val1.type));
  double data = val1.as_number * val2.as_number;
  VM_SET_TOP(vm, NUMBER_VALUE(data));
}

static inline void divide(vm_t *vm)
{
  value_t val2 = vm_pop(vm);
  value_t val1 = VM_GET_TOP(vm);
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot divide %s by %s", type_name(val1.type), type_name(val2.type));
  double data = val1.as_number / val2.as_number;
  VM_SET_TOP(vm, NUMBER_VALUE(data));
}

static inline void modulo(vm_t *vm)
{
  value_t val2 = vm_pop(vm);
  value_t val1 = VM_GET_TOP(vm);
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot mod %s by %s", type_name(val1.type), type_name(val2.type));
  double data = fmod(val1.as_number, val2.as_number);
  VM_SET_TOP(vm, NUMBER_VALUE(data));
}

static inline void negate(vm_t *vm)
{
  value_t val = VM_GET_TOP(vm);
  if (!IS_NUMBER(val))
    fatal_error("cannot apply unary minus operator to %s", type_name(val.type));
  double data = -val.as_number;
  VM_SET_TOP(vm, NUMBER_VALUE(data));
}

static inline void print(vm_t *vm)
{
  value_t val = vm_pop(vm);
  switch (val.type)
  {
  case TYPE_NULL:
    printf("null\n");
    break;
  case TYPE_NUMBER:
    printf("%g\n", val.as_number);
    break;
  }
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

void vm_init(vm_t *vm, int min_capacity)
{
  int capacity = VM_DEFAULT_NUM_SLOTS;
  while (capacity < min_capacity)
    capacity <<= 1;
  value_t *slots = (value_t *) allocate(sizeof(*slots) * capacity);
  vm->capacity = capacity;
  vm->end = capacity - 1;
  vm->slots = slots;
  vm->index = -1;
}

void vm_free(vm_t *vm)
{
  free(vm->slots);
}

void vm_push_null(vm_t *vm)
{
  push(vm, NULL_VALUE);
}

void vm_push_number(vm_t *vm, double data)
{
  push(vm, NUMBER_VALUE(data));
}

value_t vm_pop(vm_t *vm)
{
  ASSERT(vm->index > -1, "stack overflow");
  value_t val = vm->slots[vm->index];
  --vm->index;
  return val;
}

void vm_execute(vm_t *vm, chunk_t *chunk)
{
  uint8_t *pc = chunk->bytes;
  for (;;)
  {
    opcode_t op = (opcode_t) read_byte(&pc);
    switch (op)
    {
    case OP_NULL:
      vm_push_null(vm);
      break;
    case OP_INT:
      vm_push_number(vm, read_word(&pc));
      break;
    case OP_ADD:
      add(vm);
      break;
    case OP_SUBTRACT:
      subtract(vm);
      break;
    case OP_MULTIPLY:
      multiply(vm);
      break;
    case OP_DIVIDE:
      divide(vm);
      break;
    case OP_MODULO:
      modulo(vm);
      break;
    case OP_NEGATE:
      negate(vm);
      break;
    case OP_PRINT:
      print(vm);
      break;
    case OP_RETURN:
      return;
    }
  }
}
