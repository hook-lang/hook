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
static inline void function(vm_t *vm, prototype_t *proto);
static inline int unpack(vm_t *vm, int length);
static inline int append(vm_t *vm);
static inline int get_element(vm_t *vm);
static inline int fetch_element(vm_t *vm);
static inline void set_element(vm_t *vm);
static inline int put_element(vm_t *vm);
static inline int delete(vm_t *vm);
static inline int inplace_append(vm_t *vm);
static inline int inplace_put_element(vm_t *vm);
static inline int inplace_delete(vm_t *vm);
static inline void equal(vm_t *vm);
static inline int greater(vm_t *vm);
static inline int less(vm_t *vm);
static inline int add(vm_t *vm);
static inline int subtract(vm_t *vm);
static inline int multiply(vm_t *vm);
static inline int divide(vm_t *vm);
static inline int modulo(vm_t *vm);
static inline int negate(vm_t *vm);
static inline void not(vm_t *vm);
static inline int call(vm_t *vm, int num_args);
static inline int check_arity(int arity, string_t *name, int num_args);
static inline void print_trace(string_t *name, string_t *file, int line);
static inline int call_function(vm_t *vm, value_t *frame, function_t *fn, int *line);
static inline void pop_frame(vm_t *vm, value_t *frame);
static inline void move_result_and_pop_frame(vm_t *vm, value_t *frame);

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
  vm->index -= length;
  INCR_REF(arr);
  push(vm, ARRAY_VALUE(arr));
}

static inline void function(vm_t *vm, prototype_t *proto)
{
  int num_nonlocals = proto->num_nonlocals;
  value_t *slots = &vm->slots[vm->index - num_nonlocals + 1];
  function_t *fn = function_new(proto);
  for (int i = 0; i < num_nonlocals; ++i)
    fn->nonlocals[i] = slots[i];
  vm->index -= num_nonlocals;
  INCR_REF(fn);
  push(vm, FUNCTION_VALUE(fn));
}

static inline int unpack(vm_t *vm, int length)
{
  value_t *slots = &vm->slots[vm->index];
  value_t val = slots[0];
  if (!IS_ARRAY(val))
  {
    runtime_error("cannot unpack value of type '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
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
  return STATUS_OK;
}

static inline int append(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
  {
    runtime_error("cannot use '%s' as an array", type_name(val1.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val1);
  array_t *result = array_add_element(arr, val2);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  --vm->index;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val2);
  return STATUS_OK;
}

static inline int get_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
  {
    runtime_error("cannot use '%s' as an array", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val2))
  {
    runtime_error("array cannot be indexed by '%s'", type_name(val2.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
  {
    runtime_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
    return STATUS_ERROR;
  }
  value_t elem = arr->elements[index];
  VALUE_INCR_REF(elem);
  slots[0] = elem;
  --vm->index;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  return STATUS_OK;
}

static inline int fetch_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
  {
    runtime_error("cannot use '%s' as an array", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val2))
  {
    runtime_error("array cannot be indexed by '%s'", type_name(val2.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
  {
    runtime_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
    return STATUS_ERROR;
  }
  value_t elem = arr->elements[index];
  VALUE_INCR_REF(elem);
  push(vm, elem);
  return STATUS_OK;
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

static inline int put_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 2];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  value_t val3 = slots[2];
  if (!IS_ARRAY(val1))
  {
    runtime_error("cannot use '%s' as an array", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val2))
  {
    runtime_error("array cannot be indexed by '%s'", type_name(val2.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
  {
    runtime_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
    return STATUS_ERROR;
  }
  array_t *result = array_set_element(arr, index, val3);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  vm->index -= 2;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val3);
  return STATUS_OK;
}

static inline int delete(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
  {
    runtime_error("cannot use '%s' as an array", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val2))
  {
    runtime_error("array cannot be indexed by '%s'", type_name(val2.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
  {
    runtime_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
    return STATUS_ERROR;
  }
  array_t *result = array_delete_element(arr, index);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  --vm->index;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  return STATUS_OK;
}

static inline int inplace_append(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
  {
    runtime_error("cannot use '%s' as an array", type_name(val1.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val1);
  if (arr->ref_count == 2)
  {
    array_inplace_add_element(arr, val2);
    --vm->index;
    VALUE_DECR_REF(val2);
    return STATUS_OK;
  }
  array_t *result = array_add_element(arr, val2);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  --vm->index;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val2);
  return STATUS_OK;
}

static inline int inplace_put_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 2];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  value_t val3 = slots[2];
  if (!IS_ARRAY(val1))
  {
    runtime_error("cannot use '%s' as an array", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val2))
  {
    runtime_error("array cannot be indexed by '%s'", type_name(val2.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
  {
    runtime_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
    return STATUS_ERROR;
  }
  if (arr->ref_count == 2)
  {
    array_inplace_set_element(arr, index, val3);
    vm->index -= 2;
    VALUE_DECR_REF(val3);
    return STATUS_OK;
  }
  array_t *result = array_set_element(arr, index, val3);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  vm->index -= 2;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val3);
  return STATUS_OK;
}

static inline int inplace_delete(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_ARRAY(val1))
  {
    runtime_error("cannot use '%s' as an array", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val2))
  {
    runtime_error("array cannot be indexed by '%s'", type_name(val2.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  if (index < 0 || index >= arr->length)
  {
    runtime_error("index out of bounds: the length is %d but the index is %d",
      arr->length, index);
    return STATUS_ERROR;
  }
  if (arr->ref_count == 2)
  {
    array_inplace_delete_element(arr, index);
    --vm->index;
    return STATUS_OK;
  }
  array_t *result = array_delete_element(arr, index);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  --vm->index;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  return STATUS_OK;
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

static inline int greater(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  int result;
  if (value_compare(val1, val2, &result) == STATUS_ERROR)
    return STATUS_ERROR;
  slots[0] = result > 0 ? TRUE_VALUE : FALSE_VALUE;
  --vm->index;
  value_release(val1);
  value_release(val2);
  return STATUS_OK;
}

static inline int less(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  int result;
  if (value_compare(val1, val2, &result) == STATUS_ERROR)
    return STATUS_ERROR;
  slots[0] = result < 0 ? TRUE_VALUE : FALSE_VALUE;
  --vm->index;
  value_release(val1);
  value_release(val2);
  return STATUS_OK;
}

static inline int add(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  switch (val1.type)
  {
  case TYPE_NUMBER:
    {
      if (!IS_NUMBER(val2))
      {
        runtime_error("cannot add '%s' to 'number'", type_name(val2.type));
        return STATUS_ERROR;
      }
      double data = val1.as.number + val2.as.number;
      slots[0] = NUMBER_VALUE(data);
      --vm->index;
    }
    return STATUS_OK;
  case TYPE_STRING:
    {
      if (!IS_STRING(val2))
      {
        runtime_error("cannot concatenate 'string' and '%s'", type_name(val2.type));
        return STATUS_ERROR;
      }
      string_t *str1 = AS_STRING(val1);
      if (!str1->length)
      {
        slots[0] = val2;
        --vm->index;
        DECR_REF(str1);
        if (IS_UNREACHABLE(str1))
          string_free(str1);
        return STATUS_OK;
      }
      string_t *str2 = AS_STRING(val2);
      if (!str2->length)
      {
        --vm->index;
        DECR_REF(str2);
        if (IS_UNREACHABLE(str2))
          string_free(str2);
        return STATUS_OK;
      }
      if (str1->ref_count == 1)
      {
        string_inplace_concat(str1, str2);
        --vm->index;
        DECR_REF(str2);
        if (IS_UNREACHABLE(str2))
          string_free(str2);
        return STATUS_OK;
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
    return STATUS_OK;
  case TYPE_ARRAY:
    {
      if (!IS_ARRAY(val2))
      {
        runtime_error("cannot concatenate 'array' and '%s'", type_name(val2.type));
        return STATUS_ERROR;
      }
      array_t *arr1 = AS_ARRAY(val1);
      if (!arr1->length)
      {
        slots[0] = val2;
        --vm->index;
        DECR_REF(arr1);
        if (IS_UNREACHABLE(arr1))
          array_free(arr1);
        return STATUS_OK;
      }
      array_t *arr2 = AS_ARRAY(val2);
      if (!arr2->length)
      {
        --vm->index;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return STATUS_OK;
      }
      if (arr1->ref_count == 1)
      {
        array_inplace_concat(arr1, arr2);
        --vm->index;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return STATUS_OK;
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
    return STATUS_OK;
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_CALLABLE:
    break;
  }
  runtime_error("cannot add '%s' to '%s'", type_name(val2.type), type_name(val1.type));
  return STATUS_ERROR;
}

static inline int subtract(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  switch (val1.type)
  {
  case TYPE_NUMBER:
    {
      if (!IS_NUMBER(val2))
      {
        runtime_error("cannot subtract '%s' from 'number'", type_name(val2.type));
        return STATUS_ERROR;
      }
      double data = val1.as.number - val2.as.number;
      slots[0] = NUMBER_VALUE(data);
      --vm->index;
    }
    return STATUS_OK;
  case TYPE_ARRAY:
    {
      if (!IS_ARRAY(val2))
      {
        runtime_error("cannot diff between 'array' and '%s'", type_name(val2.type));
        return STATUS_ERROR;
      }
      array_t *arr1 = AS_ARRAY(val1);
      array_t *arr2 = AS_ARRAY(val2);
      if (!arr1->length || !arr2->length)
      {
        --vm->index;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return STATUS_OK;
      }
      if (arr1->ref_count == 1)
      {
        array_inplace_diff(arr1, arr2);
        --vm->index;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return STATUS_OK;
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
    return STATUS_OK;
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_STRING:
  case TYPE_CALLABLE:
    break;
  }
  runtime_error("cannot subtract '%s' from '%s'", type_name(val2.type), type_name(val1.type));
  return STATUS_ERROR;
}

static inline int multiply(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
  {
    runtime_error("cannot multiply '%s' to '%s'", type_name(val2.type), type_name(val1.type));
    return STATUS_ERROR;
  }
  double data = val1.as.number * val2.as.number;
  slots[0] = NUMBER_VALUE(data);
  --vm->index;
  return STATUS_OK;
}

static inline int divide(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
  {
    runtime_error("cannot divide '%s' by '%s'", type_name(val1.type), type_name(val2.type));
    return STATUS_ERROR;
  }
  double data = val1.as.number / val2.as.number;
  slots[0] = NUMBER_VALUE(data);
  --vm->index;
  return STATUS_OK;
}

static inline int modulo(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
  {
    runtime_error("cannot mod '%s' by '%s'", type_name(val1.type), type_name(val2.type));
    return STATUS_ERROR;
  }
  double data = fmod(val1.as.number, val2.as.number);
  slots[0] = NUMBER_VALUE(data);
  --vm->index;
  return STATUS_OK;
}

static inline int negate(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index];
  value_t val = slots[0];
  if (!IS_NUMBER(val))
  {
    runtime_error("cannot apply unary minus operator to '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  double data = -val.as.number;
  slots[0] = NUMBER_VALUE(data);
  return STATUS_OK;
}

static inline void not(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->index];
  value_t val = slots[0];
  slots[0] = IS_FALSEY(val) ? TRUE_VALUE : FALSE_VALUE;
  value_release(val);
}

static inline int call(vm_t *vm, int num_args)
{
  value_t *frame = &vm->slots[vm->index - num_args];
  value_t val = frame[0];
  if (!IS_CALLABLE(val))
  {
    runtime_error("cannot call value of type '%s'", type_name(val.type));
    pop_frame(vm, frame);
    return STATUS_ERROR;
  }
  if (IS_NATIVE(val))
  {
    native_t *native = AS_NATIVE(val);
    if (check_arity(native->arity, native->name, num_args) == STATUS_ERROR)
    {
      pop_frame(vm, frame);
      return STATUS_ERROR;
    }
    int status;
    if ((status = native->call(vm, frame)) != STATUS_OK)
    {
      if (status == STATUS_ERROR)
        print_trace(native->name, NULL, 0);
      pop_frame(vm, frame);
      return STATUS_ERROR;
    }
    DECR_REF(native);
    if (IS_UNREACHABLE(native))
      native_free(native);
    move_result_and_pop_frame(vm, frame);
    return STATUS_OK;
  }
  function_t *fn = AS_FUNCTION(val);
  prototype_t *proto = fn->proto;
  if (check_arity(proto->arity, proto->name, num_args) == STATUS_ERROR)
  {
    pop_frame(vm, frame);
    return STATUS_ERROR;
  }
  int line;
  if (call_function(vm, frame, fn, &line) == STATUS_ERROR)
  {
    print_trace(proto->name, proto->file, line);
    pop_frame(vm, frame);
    return STATUS_ERROR;
  }
  DECR_REF(fn);
  if (IS_UNREACHABLE(fn))
    function_free(fn);
  move_result_and_pop_frame(vm, frame);
  return STATUS_OK;
}

static inline int check_arity(int arity, string_t *name, int num_args)
{
  if (num_args >= arity)
    return STATUS_OK;
  const char *fmt = arity > 1 ? "%.*s() expects %d arguments but got %d" :
    "%.*s() expects %d argument but got %d";
  runtime_error(fmt, name->length, name->chars, arity, num_args);
  return STATUS_ERROR;
}

static inline void print_trace(string_t *name, string_t *file, int line)
{
  char *chars = name ? name->chars : "<anonymous>";
  if (file)
  {
    fprintf(stderr, "  at %s() in %.*s:%d\n", chars, file->length, file->chars, line);
    return;
  }
  fprintf(stderr, "  at %s() in <native>\n", chars);
}

static inline int call_function(vm_t *vm, value_t *frame, function_t *fn, int *line)
{
  value_t *slots = vm->slots;
  prototype_t *proto = fn->proto;
  value_t *nonlocals = fn->nonlocals;
  uint8_t *code = proto->chunk.bytes;
  value_t *consts = proto->consts->elements;
  prototype_t **protos = proto->protos;
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
    case OP_FUNCTION:
      function(vm, protos[read_byte(&pc)]);
      break;
    case OP_UNPACK:
      if (unpack(vm, read_byte(&pc)) == STATUS_ERROR)
        goto error;
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
    case OP_NONLOCAL:
      {
        value_t val = nonlocals[read_byte(&pc)];
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
      if (append(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_GET_ELEMENT:
      if (get_element(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_FETCH_ELEMENT:
      if (fetch_element(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_SET_ELEMENT:
      set_element(vm);
      break;
    case OP_PUT_ELEMENT:
      if (put_element(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_DELETE:
      if (delete(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_INPLACE_APPEND:
      if (inplace_append(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_INPLACE_PUT_ELEMENT:
      if (inplace_put_element(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_INPLACE_DELETE:
      if (inplace_delete(vm) == STATUS_ERROR)
        goto error;
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
      if (greater(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_LESS:
      if (less(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_ADD:
      if (add(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_SUBTRACT:
      if (subtract(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_MULTIPLY:
      if (multiply(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_DIVIDE:
      if (divide(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_MODULO:
      if (modulo(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_NEGATE:
      if (negate(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_NOT:
      not(vm);
      break;
    case OP_CALL:
      if (call(vm, read_byte(&pc)) == STATUS_ERROR)
        goto error;
      break;
    case OP_RETURN:
      return STATUS_OK;
    }
  }
error:
  *line = prototype_get_line(proto, (int) (pc - proto->chunk.bytes));
  return STATUS_ERROR;
}

static inline void pop_frame(vm_t *vm, value_t *frame)
{
  while (&vm->slots[vm->index] >= frame)
    value_release(vm->slots[vm->index--]);
}

static inline void move_result_and_pop_frame(vm_t *vm, value_t *frame)
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
  value_t *slots = &vm->slots[vm->index - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_STRING(val1))
    fatal_error("invalid type: expected string but got '%s'", type_name(val1.type));
  if (!IS_STRING(val2))
    fatal_error("invalid type: expected string but got '%s'", type_name(val2.type));
  string_t *file = AS_STRING(val1);
  string_t *source = AS_STRING(val2);
  scanner_t scan;
  scanner_init(&scan, file, source);
  prototype_t *proto = compile(&scan);
  function_t *fn = function_new(proto);
  INCR_REF(fn);
  slots[0] = FUNCTION_VALUE(fn);
  --vm->index;
  DECR_REF(file);
  DECR_REF(source);
  scanner_free(&scan);
}

int vm_call(vm_t *vm, int num_args)
{
  return call(vm, num_args);
}
