//
// Hook Programming Language
// vm.c
//

#include "vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "common.h"
#include "chunk.h"
#include "memory.h"
#include "error.h"

static inline void push(vm_t *vm, value_t val);
static inline int read_byte(uint8_t **pc);
static inline int read_word(uint8_t **pc);
static inline void array(vm_t *vm, int length);
static inline void equal(vm_t *vm);
static inline void greater(vm_t *vm);
static inline void less(vm_t *vm);
static inline void add(vm_t *vm);
static inline void subtract(vm_t *vm);
static inline void multiply(vm_t *vm);
static inline void divide(vm_t *vm);
static inline void modulo(vm_t *vm);
static inline void negate(vm_t *vm);
static inline void not(vm_t *vm);
static inline void print(vm_t *vm);

static inline void push(vm_t *vm, value_t val)
{
  if (vm->index == vm->end)
    fatal_error("stack overflow");
  ++vm->index;
  vm->slots[vm->index] = val;
}

static inline void array(vm_t *vm, int length)
{
  value_t *slots = &vm->slots[vm->index - length + 1];
  array_t *arr = array_allocate(length);
  arr->length = length;
  for (int i = 0; i < length; i++)
    arr->elements[i] = slots[i];
  INCR_REF(arr);
  slots[0] = ARRAY_VALUE(arr);
  vm->index -= length - 1;
}

static inline void equal(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  slots[0] = value_equal(val1, val2) ? TRUE_VALUE : FALSE_VALUE;
  --vm->index;
  value_release(val1);
  value_release(val2);
}

static inline void greater(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  slots[0] = value_compare(val1, val2) > 0 ? TRUE_VALUE : FALSE_VALUE;
  --vm->index;
  value_release(val1);
  value_release(val2);
}

static inline void less(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  slots[0] = value_compare(val1, val2) < 0 ? TRUE_VALUE : FALSE_VALUE;
  --vm->index;
  value_release(val1);
  value_release(val2);
}

static inline void add(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot add %s to %s", type_name(val2.type), type_name(val1.type));
  double data = val1.as_number + val2.as_number;
  slots[0] = NUMBER_VALUE(data);
  --vm->index;
}

static inline void subtract(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot subtract %s from %s", type_name(val2.type), type_name(val1.type));
  double data = val1.as_number - val2.as_number;
  slots[0] = NUMBER_VALUE(data);
  --vm->index;
}

static inline void multiply(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot multiply %s to %s", type_name(val2.type), type_name(val1.type));
  double data = val1.as_number * val2.as_number;
  slots[0] = NUMBER_VALUE(data);
  --vm->index;
}

static inline void divide(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot divide %s by %s", type_name(val1.type), type_name(val2.type));
  double data = val1.as_number / val2.as_number;
  slots[0] = NUMBER_VALUE(data);
  --vm->index;
}

static inline void modulo(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot mod %s by %s", type_name(val1.type), type_name(val2.type));
  double data = fmod(val1.as_number, val2.as_number);
  slots[0] = NUMBER_VALUE(data);
  --vm->index;
}

static inline void negate(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index];
  value_t val = slots[0];
  if (!IS_NUMBER(val))
    fatal_error("cannot apply unary minus operator to %s", type_name(val.type));
  double data = -val.as_number;
  slots[0] = NUMBER_VALUE(data);
}

static inline void not(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index];
  value_t val = slots[0];
  slots[0] = IS_FALSEY(val) ? TRUE_VALUE : FALSE_VALUE;
  value_release(val);
}

static inline void print(vm_t *vm)
{
  value_t val = vm->slots[vm->index];
  --vm->index;
  value_print(val, false);
  printf("\n");
  value_release(val);
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
  vm->capacity = capacity;
  vm->end = capacity - 1;
  vm->slots = (value_t *) allocate(sizeof(*vm->slots) * capacity);
  vm->index = -1;
}

void vm_free(vm_t *vm)
{
  while (vm->index > -1)
  {
    value_release(vm->slots[vm->index]);
    --vm->index;
  }
  free(vm->slots);
}

void vm_push_null(vm_t *vm)
{
  push(vm, NULL_VALUE);
}

void vm_push_boolean(vm_t *vm, bool data)
{
  push(vm, data ? TRUE_VALUE : FALSE_VALUE);
}

void vm_push_number(vm_t *vm, double data)
{
  push(vm, NUMBER_VALUE(data));
}

void vm_push_string(vm_t *vm, string_t *str)
{
  INCR_REF(str);
  push(vm, STRING_VALUE(str));
}

void vm_push_array(vm_t *vm, array_t *arr)
{
  INCR_REF(arr);
  push(vm, ARRAY_VALUE(arr));
}

value_t vm_pop(vm_t *vm)
{
  ASSERT(vm->index > -1, "stack overflow");
  value_t val = vm->slots[vm->index];
  --vm->index;
  VALUE_DECR_REF(val);
  return val;
}

void vm_execute(vm_t *vm, uint8_t *code, value_t *consts)
{
  value_t *frame = vm->slots;
  uint8_t *pc = code;
  for (;;)
  {
    opcode_t op = (opcode_t) read_byte(&pc);
    switch (op)
    {
    case OP_NULL:
      push(vm, NULL_VALUE);
      break;
    case OP_FALSE:
      push(vm, FALSE_VALUE);
      break;
    case OP_TRUE:
      push(vm, TRUE_VALUE);
      break;
    case OP_INT:
      push(vm, NUMBER_VALUE(read_word(&pc)));
      break;
    case OP_CONSTANT:
      {
        value_t val = consts[read_byte(&pc)];
        VALUE_INCR_REF(val);
        push(vm, val);
      }
      break;
    case OP_ARRAY:
      array(vm, read_byte(&pc));
      break;
    case OP_POP:
      value_release(vm->slots[vm->index--]);
      break;
    case OP_LOAD:
      {
        value_t val = frame[read_byte(&pc)];
        VALUE_INCR_REF(val);
        push(vm, val);
      }
      break;
    case OP_STORE:
      {
        int index = read_byte(&pc);
        value_t val = vm->slots[vm->index];
        --vm->index;
        value_release(frame[index]);
        frame[index] = val;
      }
      break;
    case OP_JUMP:
      pc = &code[read_word(&pc)];
      break;
    case OP_JUMP_IF_FALSE:
      {
        int offset = read_word(&pc);
        value_t val = vm->slots[vm->index];
        if (IS_FALSEY(val))
          pc = &code[offset];
      }
      break;
    case OP_JUMP_IF_TRUE:
      {
        int offset = read_word(&pc);
        value_t val = vm->slots[vm->index];
        if (IS_TRUTHY(val))
          pc = &code[offset];
      }
      break;
    case OP_EQUAL:
      equal(vm);
      break;
    case OP_GREATER:
      greater(vm);
      break;
    case OP_LESS:
      less(vm);
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
    case OP_NOT:
      not(vm);
      break;
    case OP_PRINT:
      print(vm);
      break;
    case OP_RETURN:
      return;
    }
  }
}
