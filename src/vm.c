//
// Hook Programming Language
// vm.c
//

#include "vm.h"
#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "struct.h"
#include "memory.h"
#include "error.h"

static inline int push(vm_t *vm, value_t val);
static inline int read_byte(uint8_t **pc);
static inline int read_word(uint8_t **pc);
static inline int array(vm_t *vm, int length);
static inline void instance(vm_t *vm);
static inline int initialize(vm_t *vm, int num_args);
static inline int function(vm_t *vm, prototype_t *proto);
static inline int unpack(vm_t *vm, int n);
static inline int destruct(vm_t *vm, int n);
static inline int add_element(vm_t *vm);
static inline int get_element(vm_t *vm);
static inline int fetch_element(vm_t *vm);
static inline void set_element(vm_t *vm);
static inline int put_element(vm_t *vm);
static inline int delete_element(vm_t *vm);
static inline int inplace_add_element(vm_t *vm);
static inline int inplace_put_element(vm_t *vm);
static inline int inplace_delete_element(vm_t *vm);
static inline int get_field(vm_t *vm);
static inline int fetch_field(vm_t *vm);
static inline void set_field(vm_t *vm);
static inline int put_field(vm_t *vm);
static inline int inplace_put_field(vm_t *vm);
static inline void equal(vm_t *vm);
static inline int greater(vm_t *vm);
static inline int less(vm_t *vm);
static inline void not_equal(vm_t *vm);
static inline int not_greater(vm_t *vm);
static inline int not_less(vm_t *vm);
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
static inline void discard_frame(vm_t *vm, value_t *frame);
static inline void move_result(vm_t *vm, value_t *frame);

static inline int push(vm_t *vm, value_t val)
{
  if (vm->top == vm->limit)
  {
    runtime_error("stack overflow");
    return STATUS_ERROR;
  }
  ++vm->top;
  vm->slots[vm->top] = val;
  return STATUS_OK;
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

static inline int array(vm_t *vm, int length)
{
  value_t *slots = &vm->slots[vm->top - length + 1];
  array_t *arr = array_allocate(length);
  arr->length = length;
  for (int i = 0; i < length; ++i)
    arr->elements[i] = slots[i];
  vm->top -= length;
  if (push(vm, ARRAY_VALUE(arr)) == STATUS_ERROR)
  {
    array_free(arr);
    return STATUS_ERROR;
  }
  INCR_REF(arr);
  return STATUS_OK;
}

static inline void instance(vm_t *vm)
{
  struct_t *ztruct = AS_STRUCT(vm->slots[vm->top]);
  int length = ztruct->length;
  value_t *slots = &vm->slots[vm->top - length];
  instance_t *inst = instance_allocate(ztruct);
  for (int i = 0; i < length; ++i)
    inst->values[i] = slots[i];
  vm->top -= length;
  INCR_REF(inst);
  slots[0] = INSTANCE_VALUE(inst);
  DECR_REF(ztruct);
}

static inline int initialize(vm_t *vm, int num_args)
{
  value_t *slots = &vm->slots[vm->top - num_args];
  value_t val = slots[0];
  if (!IS_STRUCT(val))
  {
    runtime_error("expected struct, found `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  struct_t *ztruct = AS_STRUCT(val);
  int length = ztruct->length;
  if (length != num_args)
  {
    const char *fmt = length == 1 ? 
      "initializer of %s expects %d argument but got %d" :
      "initializer of %s expects %d arguments but got %d";
    string_t *name = ztruct->name;
    char *name_chars = name ? name->chars : "<anonymous>";
    runtime_error(fmt, name_chars, length, num_args);
    return STATUS_ERROR;
  }
  instance_t *inst = instance_allocate(ztruct);
  for (int i = 0; i < length; ++i)
    inst->values[i] = slots[i + 1];
  vm->top -= length;
  INCR_REF(inst);
  slots[0] = INSTANCE_VALUE(inst);
  DECR_REF(ztruct);
  if (IS_UNREACHABLE(ztruct))
    struct_free(ztruct);
  return STATUS_OK;
}

static inline int function(vm_t *vm, prototype_t *proto)
{
  int num_nonlocals = proto->num_nonlocals;
  value_t *slots = &vm->slots[vm->top - num_nonlocals + 1];
  function_t *fn = function_new(proto);
  for (int i = 0; i < num_nonlocals; ++i)
    fn->nonlocals[i] = slots[i];
  vm->top -= num_nonlocals;
  if (push(vm, FUNCTION_VALUE(fn)) == STATUS_ERROR)
  {
    function_free(fn);
    return STATUS_ERROR;
  }
  INCR_REF(fn);
  return STATUS_OK;
}

static inline int unpack(vm_t *vm, int n)
{
  value_t val = vm->slots[vm->top];
  if (!IS_ARRAY(val))
  {
    runtime_error("cannot unpack value of type '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  array_t *arr = AS_ARRAY(val);
  --vm->top;
  int status = STATUS_OK;
  for (int i = 0; i < n && i < arr->length; ++i)
  {
    value_t elem = arr->elements[i];
    if ((status = push(vm, elem)) == STATUS_ERROR)
      goto end;
    VALUE_INCR_REF(elem);
  }
  for (int i = arr->length; i < n; ++i)
    if ((status = push(vm, NULL_VALUE)) == STATUS_ERROR)
      break;
end:
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  return status;
}

static inline int destruct(vm_t *vm, int n)
{
  value_t val = vm->slots[vm->top];
  if (!IS_INSTANCE(val))
  {
    runtime_error("cannot destructure value of type '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  instance_t *inst = AS_INSTANCE(val);
  struct_t *ztruct = inst->ztruct;
  value_t *slots = &vm->slots[vm->top - n];
  for (int i = 0; i < n; ++i)
  {
    string_t *name = AS_STRING(slots[i]);
    int index = struct_index_of(ztruct, name);
    value_t value = index == -1 ? NULL_VALUE : inst->values[index];
    VALUE_INCR_REF(value);
    DECR_REF(name);
    slots[i] = value;
  }
  --vm->top;
  DECR_REF(inst);
  if (IS_UNREACHABLE(inst))
    instance_free(inst);
  return STATUS_OK;
}

static inline int add_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
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
  --vm->top;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val2);
  return STATUS_OK;
}

static inline int get_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
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
  --vm->top;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  return STATUS_OK;
}

static inline int fetch_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
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
  if (push(vm, elem) == STATUS_ERROR)
    return STATUS_ERROR;
  VALUE_INCR_REF(elem);
  return STATUS_OK;
}

static inline void set_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 2];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  value_t val3 = slots[2];
  array_t *arr = AS_ARRAY(val1);
  long index = (long) val2.as.number;
  array_t *result = array_set_element(arr, index, val3);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  vm->top -= 2;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val3);
}

static inline int put_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 2];
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
  vm->top -= 2;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val3);
  return STATUS_OK;
}

static inline int delete_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
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
  --vm->top;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  return STATUS_OK;
}

static inline int inplace_add_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
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
    --vm->top;
    VALUE_DECR_REF(val2);
    return STATUS_OK;
  }
  array_t *result = array_add_element(arr, val2);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  --vm->top;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val2);
  return STATUS_OK;
}

static inline int inplace_put_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 2];
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
    vm->top -= 2;
    VALUE_DECR_REF(val3);
    return STATUS_OK;
  }
  array_t *result = array_set_element(arr, index, val3);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  vm->top -= 2;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  VALUE_DECR_REF(val3);
  return STATUS_OK;
}

static inline int inplace_delete_element(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
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
    --vm->top;
    return STATUS_OK;
  }
  array_t *result = array_delete_element(arr, index);
  INCR_REF(result);
  slots[0] = ARRAY_VALUE(result);
  --vm->top;
  DECR_REF(arr);
  if (IS_UNREACHABLE(arr))
    array_free(arr);
  return STATUS_OK;
}

static inline int get_field(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_INSTANCE(val1))
  {
    runtime_error("cannot use '%s' as an struct instance", type_name(val1.type));
    return STATUS_ERROR;
  }
  instance_t *inst = AS_INSTANCE(val1);
  string_t *name = AS_STRING(val2);
  int index = struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    runtime_error("no field `%.*s` on struct", name->length, name->chars);
    return STATUS_ERROR;
  }
  value_t value = inst->values[index];
  VALUE_INCR_REF(value);
  slots[0] = value;
  --vm->top;
  DECR_REF(inst);
  if (IS_UNREACHABLE(inst))
    instance_free(inst);
  DECR_REF(name);
  if (IS_UNREACHABLE(name))
    string_free(name);
  return STATUS_OK;
}

static inline int fetch_field(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_INSTANCE(val1))
  {
    runtime_error("cannot use '%s' as an instance", type_name(val1.type));
    return STATUS_ERROR;
  }
  instance_t *inst = AS_INSTANCE(val1);
  string_t *name = AS_STRING(val2);
  int index = struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    runtime_error("no field `%.*s` on struct", name->length, name->chars);
    return STATUS_ERROR;
  }
  value_t value = inst->values[index];
  if (push(vm, value) == STATUS_ERROR)
    return STATUS_ERROR;
  VALUE_INCR_REF(value);
  slots[1] = NUMBER_VALUE(index);
  DECR_REF(name);
  if (IS_UNREACHABLE(name))
    string_free(name);
  return STATUS_OK;
}

static inline void set_field(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 2];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  value_t val3 = slots[2];
  instance_t *inst = AS_INSTANCE(val1);
  int index = (int) val2.as.number;
  instance_t *result = instance_set_field(inst, index, val3);
  INCR_REF(result);
  slots[0] = INSTANCE_VALUE(result);
  vm->top -= 2;
  DECR_REF(inst);
  if (IS_UNREACHABLE(inst))
    instance_free(inst);
  VALUE_DECR_REF(val3);
}

static inline int put_field(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 2];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  value_t val3 = slots[2];
  if (!IS_INSTANCE(val1))
  {
    runtime_error("cannot use '%s' as an instance", type_name(val1.type));
    return STATUS_ERROR;
  }
  instance_t *inst = AS_INSTANCE(val1);
  string_t *name = AS_STRING(val2);
  int index = struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    runtime_error("no field `%.*s` on struct", name->length, name->chars);
    return STATUS_ERROR;
  }
  instance_t *result = instance_set_field(inst, index, val3);
  INCR_REF(result);
  slots[0] = INSTANCE_VALUE(result);
  vm->top -= 2;
  DECR_REF(inst);
  if (IS_UNREACHABLE(inst))
    instance_free(inst);
  DECR_REF(name);
  if (IS_UNREACHABLE(name))
    string_free(name);
  VALUE_DECR_REF(val3);
  return STATUS_OK;
}

static inline int inplace_put_field(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 2];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  value_t val3 = slots[2];
  if (!IS_INSTANCE(val1))
  {
    runtime_error("cannot use '%s' as an instance", type_name(val1.type));
    return STATUS_ERROR;
  }
  instance_t *inst = AS_INSTANCE(val1);
  string_t *name = AS_STRING(val2);
  int index = struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    runtime_error("no field `%.*s` on struct", name->length, name->chars);
    return STATUS_ERROR;
  }
  if (inst->ref_count == 2)
  {
    instance_inplace_set_field(inst, index, val3);
    vm->top -= 2;
    goto end;
  }
  instance_t *result = instance_set_field(inst, index, val3);
  INCR_REF(result);
  slots[0] = INSTANCE_VALUE(result);
  vm->top -= 2;
  DECR_REF(inst);
  if (IS_UNREACHABLE(inst))
    instance_free(inst);
end:
  DECR_REF(name);
  if (IS_UNREACHABLE(name))
    string_free(name);
  VALUE_DECR_REF(val3);
  return STATUS_OK;
}

static inline void equal(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  slots[0] = value_equal(val1, val2) ? TRUE_VALUE : FALSE_VALUE;
  --vm->top;
  value_release(val1);
  value_release(val2);
}

static inline int greater(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  int result;
  if (value_compare(val1, val2, &result) == STATUS_ERROR)
    return STATUS_ERROR;
  slots[0] = result > 0 ? TRUE_VALUE : FALSE_VALUE;
  --vm->top;
  value_release(val1);
  value_release(val2);
  return STATUS_OK;
}

static inline int less(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  int result;
  if (value_compare(val1, val2, &result) == STATUS_ERROR)
    return STATUS_ERROR;
  slots[0] = result < 0 ? TRUE_VALUE : FALSE_VALUE;
  --vm->top;
  value_release(val1);
  value_release(val2);
  return STATUS_OK;
}

static inline void not_equal(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  slots[0] = value_equal(val1, val2) ? FALSE_VALUE : TRUE_VALUE;
  --vm->top;
  value_release(val1);
  value_release(val2);
}

static inline int not_greater(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  int result;
  if (value_compare(val1, val2, &result) == STATUS_ERROR)
    return STATUS_ERROR;
  slots[0] = result > 0 ? FALSE_VALUE : TRUE_VALUE;
  --vm->top;
  value_release(val1);
  value_release(val2);
  return STATUS_OK;
}

static inline int not_less(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  int result;
  if (value_compare(val1, val2, &result) == STATUS_ERROR)
    return STATUS_ERROR;
  slots[0] = result < 0 ? FALSE_VALUE : TRUE_VALUE;
  --vm->top;
  value_release(val1);
  value_release(val2);
  return STATUS_OK;
}

static inline int add(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
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
      --vm->top;
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
        --vm->top;
        DECR_REF(str1);
        if (IS_UNREACHABLE(str1))
          string_free(str1);
        return STATUS_OK;
      }
      string_t *str2 = AS_STRING(val2);
      if (!str2->length)
      {
        --vm->top;
        DECR_REF(str2);
        if (IS_UNREACHABLE(str2))
          string_free(str2);
        return STATUS_OK;
      }
      if (str1->ref_count == 1)
      {
        string_inplace_concat(str1, str2);
        --vm->top;
        DECR_REF(str2);
        if (IS_UNREACHABLE(str2))
          string_free(str2);
        return STATUS_OK;
      }
      string_t *result = string_concat(str1, str2);
      INCR_REF(result);
      slots[0] = STRING_VALUE(result);
      --vm->top;
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
        --vm->top;
        DECR_REF(arr1);
        if (IS_UNREACHABLE(arr1))
          array_free(arr1);
        return STATUS_OK;
      }
      array_t *arr2 = AS_ARRAY(val2);
      if (!arr2->length)
      {
        --vm->top;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return STATUS_OK;
      }
      if (arr1->ref_count == 1)
      {
        array_inplace_concat(arr1, arr2);
        --vm->top;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return STATUS_OK;
      }
      array_t *result = array_concat(arr1, arr2);
      INCR_REF(result);
      slots[0] = ARRAY_VALUE(result);
      --vm->top;
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
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("cannot add '%s' to '%s'", type_name(val2.type), type_name(val1.type));
  return STATUS_ERROR;
}

static inline int subtract(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
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
      --vm->top;
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
        --vm->top;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return STATUS_OK;
      }
      if (arr1->ref_count == 1)
      {
        array_inplace_diff(arr1, arr2);
        --vm->top;
        DECR_REF(arr2);
        if (IS_UNREACHABLE(arr2))
          array_free(arr2);
        return STATUS_OK;
      }
      array_t *result = array_diff(arr1, arr2);
      INCR_REF(result);
      slots[0] = ARRAY_VALUE(result);
      --vm->top;
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
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("cannot subtract '%s' from '%s'", type_name(val2.type), type_name(val1.type));
  return STATUS_ERROR;
}

static inline int multiply(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
  {
    runtime_error("cannot multiply '%s' to '%s'", type_name(val2.type), type_name(val1.type));
    return STATUS_ERROR;
  }
  double data = val1.as.number * val2.as.number;
  slots[0] = NUMBER_VALUE(data);
  --vm->top;
  return STATUS_OK;
}

static inline int divide(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
  {
    runtime_error("cannot divide '%s' by '%s'", type_name(val1.type), type_name(val2.type));
    return STATUS_ERROR;
  }
  double data = val1.as.number / val2.as.number;
  slots[0] = NUMBER_VALUE(data);
  --vm->top;
  return STATUS_OK;
}

static inline int modulo(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top - 1];
  value_t val1 = slots[0];
  value_t val2 = slots[1];
  if (!IS_NUMBER(val1) || !IS_NUMBER(val2))
  {
    runtime_error("cannot mod '%s' by '%s'", type_name(val1.type), type_name(val2.type));
    return STATUS_ERROR;
  }
  double data = fmod(val1.as.number, val2.as.number);
  slots[0] = NUMBER_VALUE(data);
  --vm->top;
  return STATUS_OK;
}

static inline int negate(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top];
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
  value_t *slots = &vm->slots[vm->top];
  value_t val = slots[0];
  slots[0] = IS_FALSEY(val) ? TRUE_VALUE : FALSE_VALUE;
  value_release(val);
}

static inline int incr(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top];
  value_t val = slots[0];
  if (!IS_NUMBER(val))
  {
    runtime_error("cannot increment value of type '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  ++slots[0].as.number;
  return STATUS_OK;
}

static inline int decr(vm_t *vm)
{
  value_t *slots = &vm->slots[vm->top];
  value_t val = slots[0];
  if (!IS_NUMBER(val))
  {
    runtime_error("cannot decrement value of type '%s'", type_name(val.type));
    return STATUS_ERROR;
  }
  --slots[0].as.number;
  return STATUS_OK;
}

static inline int call(vm_t *vm, int num_args)
{
  value_t *frame = &vm->slots[vm->top - num_args];
  value_t val = frame[0];
  if (!IS_CALLABLE(val))
  {
    runtime_error("cannot call value of type '%s'", type_name(val.type));
    discard_frame(vm, frame);
    return STATUS_ERROR;
  }
  if (IS_NATIVE(val))
  {
    native_t *native = AS_NATIVE(val);
    if (check_arity(native->arity, native->name, num_args) == STATUS_ERROR)
    {
      discard_frame(vm, frame);
      return STATUS_ERROR;
    }
    int status;
    if ((status = native->call(vm, frame)) != STATUS_OK)
    {
      if (status != STATUS_NO_TRACE)
        print_trace(native->name, NULL, 0);
      discard_frame(vm, frame);
      return STATUS_ERROR;
    }
    DECR_REF(native);
    if (IS_UNREACHABLE(native))
      native_free(native);
    move_result(vm, frame);
    return STATUS_OK;
  }
  function_t *fn = AS_FUNCTION(val);
  prototype_t *proto = fn->proto;
  if (check_arity(proto->arity, proto->name, num_args) == STATUS_ERROR)
  {
    discard_frame(vm, frame);
    return STATUS_ERROR;
  }
  int line;
  if (call_function(vm, frame, fn, &line) == STATUS_ERROR)
  {
    print_trace(proto->name, proto->file, line);
    discard_frame(vm, frame);
    return STATUS_ERROR;
  }
  DECR_REF(fn);
  if (IS_UNREACHABLE(fn))
    function_free(fn);
  move_result(vm, frame);
  return STATUS_OK;
}

static inline int check_arity(int arity, string_t *name, int num_args)
{
  if (num_args >= arity)
    return STATUS_OK;
  const char *fmt = arity == 1 ? "%s() expects %d argument but got %d" :
    "%s() expects %d arguments but got %d";
  char *name_chars = name ? name->chars : "<anonymous>";
  runtime_error(fmt, name_chars, arity, num_args);
  return STATUS_ERROR;
}

static inline void print_trace(string_t *name, string_t *file, int line)
{
  char *name_chars = name ? name->chars : "<anonymous>";
  if (file)
  {
    fprintf(stderr, "  at %s() in %.*s:%d\n", name_chars, file->length, file->chars, line);
    return;
  }
  fprintf(stderr, "  at %s() in <native>\n", name_chars);
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
      if (push(vm, NULL_VALUE) == STATUS_ERROR)
        goto error;
      break;
    case OP_FALSE:
      if (push(vm, FALSE_VALUE) == STATUS_ERROR)
        goto error;
      break;
    case OP_TRUE:
      if (push(vm, TRUE_VALUE) == STATUS_ERROR)
        goto error;
      break;
    case OP_INT:
      if (push(vm, NUMBER_VALUE(read_word(&pc))) == STATUS_ERROR)
        goto error;
      break;
    case OP_CONSTANT:
      {
        value_t val = consts[read_byte(&pc)];
        if (push(vm, val) == STATUS_ERROR)
          goto error;
        VALUE_INCR_REF(val);
      }
      break;
    case OP_ARRAY:
      if (array(vm, read_byte(&pc)) == STATUS_ERROR)
        goto error;
      break;
    case OP_INSTANCE:
      instance(vm);
      break;
    case OP_INITILIZE:
      if (initialize(vm, read_byte(&pc)) == STATUS_ERROR)
        goto error;
      break;
    case OP_FUNCTION:
      if (function(vm, protos[read_byte(&pc)]) == STATUS_ERROR)
        goto error;
      break;
    case OP_UNPACK:
      if (unpack(vm, read_byte(&pc)) == STATUS_ERROR)
        goto error;
      break;
    case OP_DESTRUCT:
      if (destruct(vm, read_byte(&pc)) == STATUS_ERROR)
        goto error;
      break;
    case OP_POP:
      value_release(slots[vm->top--]);
      break;
    case OP_GLOBAL:
      {
        value_t val = slots[read_byte(&pc)];
        if (push(vm, val) == STATUS_ERROR)
          goto error;
        VALUE_INCR_REF(val);
      }
      break;
    case OP_NONLOCAL:
      {
        value_t val = nonlocals[read_byte(&pc)];
        if (push(vm, val) == STATUS_ERROR)
          goto error;
        VALUE_INCR_REF(val);
      }
      break;
    case OP_GET_LOCAL:
      {
        value_t val = frame[read_byte(&pc)];
        if (push(vm, val) == STATUS_ERROR)
          goto error;
        VALUE_INCR_REF(val);
      }
      break;
    case OP_SET_LOCAL:
      {
        int index = read_byte(&pc);
        value_t val = slots[vm->top];
        --vm->top;
        value_release(frame[index]);
        frame[index] = val;
      }
      break;
    case OP_ADD_ELEMENT:
      if (add_element(vm) == STATUS_ERROR)
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
    case OP_DELETE_ELEMENT:
      if (delete_element(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_INPLACE_ADD_ELEMENT:
      if (inplace_add_element(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_INPLACE_PUT_ELEMENT:
      if (inplace_put_element(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_INPLACE_DELETE_ELEMENT:
      if (inplace_delete_element(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_GET_FIELD:
      if (get_field(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_FETCH_FIELD:
      if (fetch_field(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_SET_FIELD:
      set_field(vm);
      break;
    case OP_PUT_FIELD:
      if (put_field(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_INPLACE_PUT_FIELD:
      if (inplace_put_field(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_JUMP:
      pc = &code[read_word(&pc)];
      break;
    case OP_JUMP_IF_FALSE:
      {
        int offset = read_word(&pc);
        value_t val = slots[vm->top];
        if (IS_FALSEY(val))
          pc = &code[offset];
      }
      break;
    case OP_JUMP_IF_TRUE:
      {
        int offset = read_word(&pc);
        value_t val = slots[vm->top];
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
    case OP_NOT_EQUAL:
      not_equal(vm);
      break;
    case OP_NOT_GREATER:
      if (not_greater(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_NOT_LESS:
      if (not_less(vm) == STATUS_ERROR)
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
    case OP_INCR:
      if (incr(vm) == STATUS_ERROR)
        goto error;
      break;
    case OP_DECR:
      if (decr(vm) == STATUS_ERROR)
        goto error;
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

static inline void discard_frame(vm_t *vm, value_t *frame)
{
  while (&vm->slots[vm->top] >= frame)
    value_release(vm->slots[vm->top--]);
}

static inline void move_result(vm_t *vm, value_t *frame)
{
  frame[0] = vm->slots[vm->top];
  --vm->top;
  while (&vm->slots[vm->top] > frame)
    value_release(vm->slots[vm->top--]);
}

void vm_init(vm_t *vm, int min_capacity)
{
  int capacity = VM_MIN_CAPACITY;
  while (capacity < min_capacity)
    capacity <<= 1;
  vm->capacity = capacity;
  vm->limit = capacity - 1;
  vm->slots = (value_t *) allocate(sizeof(*vm->slots) * capacity);
  vm->top = -1;
}

void vm_free(vm_t *vm)
{
  while (vm->top > -1)
    value_release(vm->slots[vm->top--]);
  free(vm->slots);
}

int vm_push_value(vm_t *vm, value_t val)
{
  if (push(vm, val) == STATUS_ERROR)
    return STATUS_ERROR;
  VALUE_INCR_REF(val);
  return STATUS_OK;
}

int vm_push_null(vm_t *vm)
{
  return push(vm, NULL_VALUE);
}

int vm_push_boolean(vm_t *vm, bool data)
{
  return push(vm, data ? TRUE_VALUE : FALSE_VALUE);
}

int vm_push_number(vm_t *vm, double data)
{
  return push(vm, NUMBER_VALUE(data));
}

int vm_push_string(vm_t *vm, string_t *str)
{
  return vm_push_value(vm, STRING_VALUE(str));
}

int vm_push_array(vm_t *vm, array_t *arr)
{
  return vm_push_value(vm, ARRAY_VALUE(arr));
}

int vm_push_struct(vm_t *vm, struct_t *ztruct)
{
  return vm_push_value(vm, STRUCT_VALUE(ztruct));
}

int vm_push_instance(vm_t *vm, instance_t *inst)
{
  return vm_push_value(vm, INSTANCE_VALUE(inst));
}

int vm_push_function(vm_t *vm, function_t *fn)
{
  return vm_push_value(vm, FUNCTION_VALUE(fn));
}

int vm_push_native(vm_t *vm, native_t *native)
{
  return vm_push_value(vm, NATIVE_VALUE(native));
}

int vm_push_userdata(vm_t *vm, userdata_t *udata)
{
  return vm_push_value(vm, USERDATA_VALUE(udata));
}

void vm_pop(vm_t *vm)
{
  ASSERT(vm->top > -1, "stack underflow");
  value_t val = vm->slots[vm->top];
  --vm->top;
  value_release(val);
}

void vm_instance(vm_t *vm)
{
  instance(vm);
}

int vm_initialize(vm_t *vm, int num_args)
{
  return initialize(vm, num_args);
}

int vm_call(vm_t *vm, int num_args)
{
  return call(vm, num_args);
}
