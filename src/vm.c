//
// Hook Programming Language
// vm.c
//

#include "vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "common.h"
#include "compiler.h"
#include "builtin.h"
#include "memory.h"
#include "error.h"

static inline void push(vm_t *vm, value_t val);
static inline int read_byte(uint8_t **pc);
static inline int read_word(uint8_t **pc);
static inline void array(vm_t *vm, int length);
static inline void unpack(vm_t *vm, int length);
static inline void append(vm_t *vm);
static inline void get_element(vm_t *vm);
static inline void fetch_element(vm_t *vm);
static inline void set_element(vm_t *vm);
static inline void put_element(vm_t *vm);
static inline void delete(vm_t *vm);
static inline void inplace_append(vm_t *vm);
static inline void inplace_put_element(vm_t *vm);
static inline void inplace_delete(vm_t *vm);
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
static inline void call(vm_t *vm, int nargs);
static inline void check_arity(callable_t *callable, int nargs);
static inline void function_call(vm_t *vm, value_t *frame, uint8_t *code, value_t *consts);
static inline void pop_frame(vm_t *vm, value_t *frame);

static inline void push(vm_t *vm, value_t val)
{
  if (vm->index == vm->end)
    fatal_error("stack overflow");
  ++vm->index;
  vm->slots[vm->index] = val;
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

static inline void array(vm_t *vm, int length)
{
  value_t *slots = &vm->slots[vm->index - length + 1];
  array_t *arr = array_allocate(length);
  arr->length = length;
  for (int i = 0; i < length; ++i)
    arr->elements[i] = slots[i];
  INCR_REF(arr);
  slots[0] = ARRAY_VALUE(arr);
  vm->index -= length - 1;
}

static inline void unpack(vm_t *vm, int length)
{
  value_t *slots = &vm->slots[vm->index];
  value_t val = slots[0];
  if (!IS_ARRAY(val))
    fatal_error("cannot unpack value of type '%s'", type_name(val.type));
  array_t *arr = AS_ARRAY(val);
  --vm->index;
  for (int i = 0; i < length && i < arr->length; ++i)
  {
    value_t elem = arr->elements[i];
    VALUE_INCR_REF(elem);
    push(vm, elem);
  }
  for (int i = arr->length; i < length; ++i)
    push(vm, NULL_VALUE);
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
}

static inline void append(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
    fatal_error("cannot use '%s' as an array", type_name(val1.type));
  array_t *arr = AS_ARRAY(val1);
  array_t *result = array_add_element(arr, val2);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  --vm->index;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val2);
}

static inline void get_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
    fatal_error("cannot use '%s' as an array", type_name(val1.type));
  if (!IS_INTEGER(val2))
    fatal_error("array cannot be indexed by '%s'", type_name(val2.type));
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
    fatal_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
  value_t elem = arr->elements[index];
  VALUE_INCR_REF(elem);
  slots[0] = elem;
  --vm->index;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
}

static inline void fetch_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
    fatal_error("cannot use '%s' as an array", type_name(val1.type));
  if (!IS_INTEGER(val2))
    fatal_error("array cannot be indexed by '%s'", type_name(val2.type));
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
    fatal_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
  value_t elem = arr->elements[index];
  VALUE_INCR_REF(elem);
  push(vm, elem);
}

static inline void set_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 2];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  value_t val3 = slots[2];
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  array_t *result = array_set_element(arr, index, val3);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  vm->index -= 2;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val3);
}

static inline void put_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 2];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  value_t val3 = slots[2];
  if (!IS_ARRAY(val1))
    fatal_error("cannot use '%s' as an array", type_name(val1.type));
  if (!IS_INTEGER(val2))
    fatal_error("array cannot be indexed by '%s'", type_name(val2.type));
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
    fatal_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
  array_t *result = array_set_element(arr, index, val3);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  vm->index -= 2;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val3);
}

static inline void delete(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
    fatal_error("cannot use '%s' as an array", type_name(val1.type));
  if (!IS_INTEGER(val2))
    fatal_error("array cannot be indexed by '%s'", type_name(val2.type));
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
    fatal_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
  array_t *result = array_delete_element(arr, index);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  --vm->index;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
}

static inline void inplace_append(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
    fatal_error("cannot use '%s' as an array", type_name(val1.type));
  array_t *arr = AS_ARRAY(val1);
  if (arr->ref_count == 2)
  {
    array_inplace_add_element(arr, val2);
    --vm->index;
    VALUE_DECR_REF(val2);
    return;
  }
  array_t *result = array_add_element(arr, val2);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  --vm->index;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val2);
}

static inline void inplace_put_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 2];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  value_t val3 = slots[2];
  if (!IS_ARRAY(val1))
    fatal_error("cannot use '%s' as an array", type_name(val1.type));
  if (!IS_INTEGER(val2))
    fatal_error("array cannot be indexed by '%s'", type_name(val2.type));
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
    fatal_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
  if (arr->ref_count == 2)
  {
    array_inplace_set_element(arr, index, val3);
    vm->index -= 2;
    VALUE_DECR_REF(val3);
    return;
  }
  array_t *result = array_set_element(arr, index, val3);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  vm->index -= 2;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val3);
}

static inline void inplace_delete(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
    fatal_error("cannot use '%s' as an array", type_name(val1.type));
  if (!IS_INTEGER(val2))
    fatal_error("array cannot be indexed by '%s'", type_name(val2.type));
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
    fatal_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
  if (arr->ref_count == 2)
  {
    array_inplace_delete_element(arr, index);
    --vm->index;
    return;
  }
  array_t *result = array_delete_element(arr, index);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  --vm->index;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
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
  switch (val1.type)
  {
  case TYPE_NUMBER:
    {
      if (!IS_NUMBER(val2))
        fatal_error("cannot add '%s' to 'number'", type_name(val2.type));
      double data = val1.as.number + val2.as.number;
      slots[0] = NUMBER_VALUE(data);
      --vm->index;
    }
    return;
  case TYPE_STRING:
    {
      if (!IS_STRING(val2))
        fatal_error("cannot concatenate 'string' and '%s'", type_name(val2.type));
      string_t *str1 = AS_STRING(val1);
      if (!str1->length)
      {
        slots[0] = val2;
        --vm->index;
        DECR_REF(str1);
        if (IS_UNREACHABLE(str1))
          string_free(str1);
        return;
      }
      string_t *str2 = AS_STRING(val2);
      if (!str2->length)
      {
        --vm->index;
        DECR_REF(str2);
        if (IS_UNREACHABLE(str2))
          string_free(str2);
        return;
      }
      if (str1->ref_count == 1)
      {
        string_inplace_concat(str1, str2);
        --vm->index;
        DECR_REF(str2);
        if (IS_UNREACHABLE(str2))
          string_free(str2);
        return;
      }
      string_t *result = string_concat(str1, str2);
      INCR_REF(result);
      slots[0] = STRING_VALUE(result);
      --vm->index;
      DECR_REF(str1);
      if (IS_UNREACHABLE(str1))
        string_free(str1);
      DECR_REF(str2);
      if (IS_UNREACHABLE(str2))
        string_free(str2);
    }
    return;
  case TYPE_ARRAY:
    {
      if (!IS_ARRAY(val2))
        fatal_error("cannot concatenate 'array' and '%s'", type_name(val2.type));
      array_t *arr1 = AS_ARRAY(val1);
      if (!arr1->length)
      {
        slots[0] = val2;
        --vm->index;
        DECR_REF(arr1);
        if (IS_UNREACHABLE(arr1))
          array_free(arr1);
        return;
      }
      array_t *arr2 = AS_ARRAY(val2);
      if (!arr2->length)
      {
        --vm->index;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return;
      }
      if (arr1->ref_count == 1)
      {
        array_inplace_concat(arr1, arr2);
        --vm->index;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return;
      }
      array_t *result = array_concat(arr1, arr2);
      INCR_REF(result);
      slots[0] = ARRAY_VALUE(result);
      --vm->index;
      DECR_REF(arr1);
      if (IS_UNREACHABLE(arr1))
        array_free(arr1);
      DECR_REF(arr2);
      if (IS_UNREACHABLE(arr2))
        array_free(arr2);
    }
    return;
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_CALLABLE:
    break;
  }
  fatal_error("cannot add '%s' to '%s'", type_name(val2.type), type_name(val1.type));
}

static inline void subtract(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  switch (val1.type)
  {
  case TYPE_NUMBER:
    {
      if (!IS_NUMBER(val2))
        fatal_error("cannot subtract '%s' from 'number'", type_name(val2.type));
      double data = val1.as.number - val2.as.number;
      slots[0] = NUMBER_VALUE(data);
      --vm->index;
    }
    return;
  case TYPE_ARRAY:
    {
      if (!IS_ARRAY(val2))
        fatal_error("cannot diff between 'array' and '%s'", type_name(val2.type));
      array_t *arr1 = AS_ARRAY(val1);
      array_t *arr2 = AS_ARRAY(val2);
      if (!arr1->length || !arr2->length)
      {
        --vm->index;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return;
      }
      if (arr1->ref_count == 1)
      {
        array_inplace_diff(arr1, arr2);
        --vm->index;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return;
      }
      array_t *result = array_diff(arr1, arr2);
      INCR_REF(result);
      slots[0] = ARRAY_VALUE(result);
      --vm->index;
      DECR_REF(arr1);
      if (IS_UNREACHABLE(arr1))
        array_free(arr1);
      DECR_REF(arr2);
      if (IS_UNREACHABLE(arr2))
        array_free(arr2);
    }
    return;
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_STRING:
  case TYPE_CALLABLE:
    break;
  }
  fatal_error("cannot subtract '%s' from '%s'", type_name(val2.type), type_name(val1.type));
}

static inline void multiply(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot multiply '%s' to '%s'", type_name(val2.type), type_name(val1.type));
  double data = val1.as.number * val2.as.number;
  slots[0] = NUMBER_VALUE(data);
  --vm->index;
}

static inline void divide(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot divide '%s' by '%s'", type_name(val1.type), type_name(val2.type));
  double data = val1.as.number / val2.as.number;
  slots[0] = NUMBER_VALUE(data);
  --vm->index;
}

static inline void modulo(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
    fatal_error("cannot mod '%s' by '%s'", type_name(val1.type), type_name(val2.type));
  double data = fmod(val1.as.number, val2.as.number);
  slots[0] = NUMBER_VALUE(data);
  --vm->index;
}

static inline void negate(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index];
  value_t val = slots[0];
  if (!IS_NUMBER(val))
    fatal_error("cannot apply unary minus operator to '%s'", type_name(val.type));
  double data = -val.as.number;
  slots[0] = NUMBER_VALUE(data);
}

static inline void not(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index];
  value_t val = slots[0];
  slots[0] = IS_FALSEY(val) ? TRUE_VALUE : FALSE_VALUE;
  value_release(val);
}

static inline void call(vm_t *vm, int nargs)
{
  value_t *frame = &vm->slots[vm->index - nargs];
  value_t val = frame[0];
  if (IS_NATIVE(val))
  {
    callable_t *callable = AS_CALLABLE(val);
    check_arity(callable, nargs);
    native_t *native = (native_t *) callable;
    native->call(vm, frame);
    DECR_REF(native);
    if (IS_UNREACHABLE(native))
      native_free(native);
    pop_frame(vm, frame);
    return;
  }
  if (!IS_CALLABLE(val))
    fatal_error("cannot call value of type '%s'", type_name(val.type));
  callable_t *callable = AS_CALLABLE(val);
  check_arity(callable, nargs);
  function_t *fn = (function_t *) callable;
  function_call(vm, frame, fn->chunk.bytes, fn->consts->elements);
  DECR_REF(fn);
  if (IS_UNREACHABLE(fn))
    function_free(fn);
  pop_frame(vm, frame);
}

static inline void check_arity(callable_t *callable, int nargs)
{
  int arity = callable->arity;
  if (nargs >= arity)
    return;
  string_t *name = callable->name;
  const char *fmt = arity > 1 ? "%.*s() expects %d arguments but got %d" :
    "%.*s() expects %d argument but got %d";
  fatal_error(fmt, name->length, name->chars, arity, nargs);
}

static inline void function_call(vm_t *vm, value_t *frame, uint8_t *code, value_t *consts)
{
  value_t *slots = vm->slots;
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
    case OP_UNPACK:
      unpack(vm, read_byte(&pc));
      break;
    case OP_POP:
      value_release(slots[vm->index--]);
      break;
    case OP_GLOBAL:
      {
        value_t val = slots[read_byte(&pc)];
        VALUE_INCR_REF(val);
        push(vm, val);
      }
      break;
    case OP_GET_LOCAL:
      {
        value_t val = frame[read_byte(&pc)];
        VALUE_INCR_REF(val);
        push(vm, val);
      }
      break;
    case OP_SET_LOCAL:
      {
        int index = read_byte(&pc);
        value_t val = slots[vm->index];
        --vm->index;
        value_release(frame[index]);
        frame[index] = val;
      }
      break;
    case OP_APPEND:
      append(vm);
      break;
    case OP_GET_ELEMENT:
      get_element(vm);
      break;
    case OP_FETCH_ELEMENT:
      fetch_element(vm);
      break;
    case OP_SET_ELEMENT:
      set_element(vm);
      break;
    case OP_PUT_ELEMENT:
      put_element(vm);
      break;
    case OP_DELETE:
      delete(vm);
      break;
    case OP_INPLACE_APPEND:
      inplace_append(vm);
      break;
    case OP_INPLACE_PUT_ELEMENT:
      inplace_put_element(vm);
      break;
    case OP_INPLACE_DELETE:
      inplace_delete(vm);
      break;
    case OP_JUMP:
      pc = &code[read_word(&pc)];
      break;
    case OP_JUMP_IF_FALSE:
      {
        int offset = read_word(&pc);
        value_t val = slots[vm->index];
        if (IS_FALSEY(val))
          pc = &code[offset];
      }
      break;
    case OP_JUMP_IF_TRUE:
      {
        int offset = read_word(&pc);
        value_t val = slots[vm->index];
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
    case OP_CALL:
      call(vm, read_byte(&pc));
      break;
    case OP_RETURN:
      return;
    }
  }
}

static inline void pop_frame(vm_t *vm, value_t *frame)
{
  frame[0] = vm->slots[vm->index];
  --vm->index;
  while (&vm->slots[vm->index] > frame)
    value_release(vm->slots[vm->index--]);
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
  globals_init(vm);
}

void vm_free(vm_t *vm)
{
  while (vm->index > -1)
    value_release(vm->slots[vm->index--]);
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

void vm_push_function(vm_t *vm, function_t *fn)
{
  INCR_REF(fn);
  push(vm, FUNCTION_VALUE(fn));
}

void vm_push_native(vm_t *vm, native_t *native)
{
  INCR_REF(native);
  push(vm, NATIVE_VALUE(native));
}

void vm_pop(vm_t *vm)
{
  ASSERT(vm->index > -1, "stack underflow");
  value_t val = vm->slots[vm->index];
  --vm->index;
  value_release(val);
}

void vm_compile(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index];
  value_t val = slots[0];
  if (!IS_STRING(val))
    fatal_error("value is not string");
  string_t *str = AS_STRING(val);
  scanner_t scan;
  scanner_init(&scan, str->chars);
  function_t *fn = compile(&scan);
  INCR_REF(fn);
  slots[0] = FUNCTION_VALUE(fn);
  DECR_REF(str);
  if (IS_UNREACHABLE(str))
    string_free(str);
}

void vm_call(vm_t *vm, int nargs)
{
  call(vm, nargs);
}
