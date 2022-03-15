//
// Hook Programming Language
// hook_vm.c
//

#include "hook_vm.h"
#include <stdlib.h>
#include <math.h>
#include "hook_builtin.h"
#include "hook_struct.h"
#include "hook_module.h"
#include "hook_utils.h"
#include "hook_memory.h"
#include "hook_status.h"
#include "hook_error.h"

static inline void type_error(int index, int num_types, int types[], int val_type);
static inline int push(hk_vm_t *vm, hk_value_t val);
static inline int read_byte(uint8_t **pc);
static inline int read_word(uint8_t **pc);
static inline int range(hk_vm_t *vm);
static inline int array(hk_vm_t *vm, int length);
static inline int ztruct(hk_vm_t *vm, int length);
static inline int instance(hk_vm_t *vm, int length);
static inline int construct(hk_vm_t *vm, int length);
static inline int closure(hk_vm_t *vm, hk_function_t *fn);
static inline int unpack(hk_vm_t *vm, int n);
static inline int destruct(hk_vm_t *vm, int n);
static inline int add_element(hk_vm_t *vm);
static inline int get_element(hk_vm_t *vm);
static inline int slice_array(hk_vm_t *vm, hk_value_t *slots, hk_array_t *arr, hk_range_t *range);
static inline int fetch_element(hk_vm_t *vm);
static inline void set_element(hk_vm_t *vm);
static inline int put_element(hk_vm_t *vm);
static inline int delete_element(hk_vm_t *vm);
static inline int inplace_add_element(hk_vm_t *vm);
static inline int inplace_put_element(hk_vm_t *vm);
static inline int inplace_delete_element(hk_vm_t *vm);
static inline int get_field(hk_vm_t *vm, hk_string_t *name);
static inline int fetch_field(hk_vm_t *vm, hk_string_t *name);
static inline void set_field(hk_vm_t *vm);
static inline int put_field(hk_vm_t *vm, hk_string_t *name);
static inline int inplace_put_field(hk_vm_t *vm, hk_string_t *name);
static inline void equal(hk_vm_t *vm);
static inline int greater(hk_vm_t *vm);
static inline int less(hk_vm_t *vm);
static inline void not_equal(hk_vm_t *vm);
static inline int not_greater(hk_vm_t *vm);
static inline int not_less(hk_vm_t *vm);
static inline int add(hk_vm_t *vm);
static inline int concat_strings(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2);
static inline int concat_arrays(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2);
static inline int subtract(hk_vm_t *vm);
static inline int diff_arrays(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2);
static inline int multiply(hk_vm_t *vm);
static inline int divide(hk_vm_t *vm);
static inline int modulo(hk_vm_t *vm);
static inline int negate(hk_vm_t *vm);
static inline void not(hk_vm_t *vm);
static inline int call(hk_vm_t *vm, int num_args);
static inline int check_arity(int arity, hk_string_t *name, int num_args);
static inline void print_trace(hk_string_t *name, hk_string_t *file, int line);
static inline int call_function(hk_vm_t *vm, hk_value_t *locals, hk_closure_t *cl, int *line);
static inline void discard_frame(hk_vm_t *vm, hk_value_t *slots);
static inline void move_result(hk_vm_t *vm, hk_value_t *slots);

static inline void type_error(int index, int num_types, int types[], int val_type)
{
  hk_assert(num_types > 0, "num_types must be greater than 0");
  fprintf(stderr, "runtime error: type error: argument #%d must be of the type %s",
    index, hk_type_name(types[0]));
  for (int i = 1; i < num_types; ++i)
    fprintf(stderr, "|%s", hk_type_name(types[i]));
  fprintf(stderr, ", %s given\n", hk_type_name(val_type));
}

static inline int push(hk_vm_t *vm, hk_value_t val)
{
  if (vm->top == vm->limit)
  {
    hk_runtime_error("stack overflow");
    return HK_STATUS_ERROR;
  }
  ++vm->top;
  vm->slots[vm->top] = val;
  return HK_STATUS_OK;
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

static inline int range(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: range must be of type number");
    return HK_STATUS_ERROR;
  }
  hk_range_t *range = hk_range_new(val1.as.number, val2.as.number);
  hk_incr_ref(range);
  slots[0] = hk_range_value(range);
  --vm->top;
  return HK_STATUS_OK;
}

static inline int array(hk_vm_t *vm, int length)
{
  hk_value_t *slots = &vm->slots[vm->top - length + 1];
  hk_array_t *arr = hk_array_allocate(length);
  arr->length = length;
  for (int i = 0; i < length; ++i)
    arr->elements[i] = slots[i];
  vm->top -= length;
  if (push(vm, hk_array_value(arr)) == HK_STATUS_ERROR)
  {
    hk_array_free(arr);
    return HK_STATUS_ERROR;
  }
  hk_incr_ref(arr);
  return HK_STATUS_OK;
}

static inline int ztruct(hk_vm_t *vm, int length)
{
  hk_value_t *slots = &vm->slots[vm->top - length];
  hk_value_t val = slots[0];
  hk_string_t *struct_name = hk_is_nil(val) ? NULL : hk_as_string(val);
  hk_struct_t *ztruct = hk_struct_new(struct_name);
  for (int i = 1; i <= length; ++i)
  {
    hk_string_t *field_name = hk_as_string(slots[i]);
    if (!hk_struct_define_field(ztruct, field_name))
    {
      hk_runtime_error("field %.*s is already defined", field_name->length,
        field_name->chars);
      hk_struct_free(ztruct);
      return HK_STATUS_ERROR;
    }
  }
  for (int i = 1; i <= length; ++i)
    hk_decr_ref(hk_as_object(slots[i]));
  vm->top -= length;
  hk_incr_ref(ztruct);
  slots[0] = hk_struct_value(ztruct);
  if (struct_name)
    hk_decr_ref(struct_name);
  return HK_STATUS_OK;
}

static inline int instance(hk_vm_t *vm, int length)
{
  hk_value_t *slots = &vm->slots[vm->top - length];
  hk_value_t val = slots[0];
  if (!hk_is_struct(val))
  {
    hk_runtime_error("type error: cannot use %s as a struct", hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  hk_struct_t *ztruct = hk_as_struct(val);
  if (ztruct->length > length)
  {
    int n = ztruct->length - length;
    const char *fmt = n == 1 ? 
      "missing %d value in initializer of %s" :
      "missing %d values in initializer of %s";  
    hk_string_t *name = ztruct->name;
    hk_runtime_error(fmt, n, name ? name->chars : "<anonymous>");
    return HK_STATUS_ERROR;
  }
  if (ztruct->length < length)
  {
    const char *fmt = "too many values in initializer of %s";
    hk_string_t *name = ztruct->name;
    hk_runtime_error(fmt, name ? name->chars : "<anonymous>");
    return HK_STATUS_ERROR;
  }
  hk_instance_t *inst = hk_instance_allocate(ztruct);
  for (int i = 0; i < length; ++i)
    inst->values[i] = slots[i + 1];
  vm->top -= length;
  hk_incr_ref(inst);
  slots[0] = hk_instance_value(inst);
  hk_struct_release(ztruct);
  return HK_STATUS_OK;
}

static inline int construct(hk_vm_t *vm, int length)
{
  int n = length << 1;
  hk_value_t *slots = &vm->slots[vm->top - n];
  hk_value_t val = slots[0];
  hk_string_t *struct_name = hk_is_nil(val) ? NULL : hk_as_string(val);
  hk_struct_t *ztruct = hk_struct_new(struct_name);
  for (int i = 1; i <= n; i += 2)
  {
    hk_string_t *field_name = hk_as_string(slots[i]);
    if (!hk_struct_define_field(ztruct, field_name))
    {
      hk_runtime_error("field %.*s is already defined", field_name->length,
        field_name->chars);
      hk_struct_free(ztruct);
      return HK_STATUS_ERROR;
    }
  }
  for (int i = 1; i <= n; i += 2)
    hk_decr_ref(hk_as_object(slots[i]));
  hk_instance_t *inst = hk_instance_allocate(ztruct);
  for (int i = 2, j = 0; i <= n + 1; i += 2, ++j)
    inst->values[j] = slots[i];
  vm->top -= n;
  hk_incr_ref(inst);
  slots[0] = hk_instance_value(inst);
  if (struct_name)
    hk_decr_ref(struct_name);
  return HK_STATUS_OK;
}

static inline int closure(hk_vm_t *vm, hk_function_t *fn)
{
  int num_nonlocals = fn->num_nonlocals;
  hk_value_t *slots = &vm->slots[vm->top - num_nonlocals + 1];
  hk_closure_t *cl = hk_closure_new(fn);
  for (int i = 0; i < num_nonlocals; ++i)
    cl->nonlocals[i] = slots[i];
  vm->top -= num_nonlocals;
  if (push(vm, hk_closure_value(cl)) == HK_STATUS_ERROR)
  {
    hk_closure_free(cl);
    return HK_STATUS_ERROR;
  }
  hk_incr_ref(cl);
  return HK_STATUS_OK;
}

static inline int unpack(hk_vm_t *vm, int n)
{
  hk_value_t val = vm->slots[vm->top];
  if (!hk_is_array(val))
  {
    hk_runtime_error("type error: cannot unpack value of type %s",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val);
  --vm->top;
  int status = HK_STATUS_OK;
  for (int i = 0; i < n && i < arr->length; ++i)
  {
    hk_value_t elem = arr->elements[i];
    if ((status = push(vm, elem)) == HK_STATUS_ERROR)
      goto end;
    hk_value_incr_ref(elem);
  }
  for (int i = arr->length; i < n; ++i)
    if ((status = push(vm, HK_NIL_VALUE)) == HK_STATUS_ERROR)
      break;
end:
  hk_array_release(arr);
  return status;
}

static inline int destruct(hk_vm_t *vm, int n)
{
  hk_value_t val = vm->slots[vm->top];
  if (!hk_is_instance(val))
  {
    hk_runtime_error("type error: cannot destructure value of type %s",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  hk_instance_t *inst = hk_as_instance(val);
  hk_struct_t *ztruct = inst->ztruct;
  hk_value_t *slots = &vm->slots[vm->top - n];
  for (int i = 0; i < n; ++i)
  {
    hk_string_t *name = hk_as_string(slots[i]);
    int index = hk_struct_index_of(ztruct, name);
    hk_value_t value = index == -1 ? HK_NIL_VALUE : inst->values[index];
    hk_value_incr_ref(value);
    hk_decr_ref(name);
    slots[i] = value;
  }
  --vm->top;
  hk_instance_release(inst);
  return HK_STATUS_OK;
}

static inline int add_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  hk_array_t *result = hk_array_add_element(arr, val2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->top;
  hk_array_release(arr);  
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline int get_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  if (hk_is_int(val2))
  {
    int index = (int) val2.as.number;
    if (index < 0 || index >= arr->length)
    {
      hk_runtime_error("range error: index %d is out of bounds for array of length %d",
        index, arr->length);
      return HK_STATUS_ERROR;
    }
    hk_value_t elem = arr->elements[index];
    hk_value_incr_ref(elem);
    slots[0] = elem;
    --vm->top;
    hk_array_release(arr);
    return HK_STATUS_OK;
  }
  if (hk_is_range(val2))
    return slice_array(vm, slots, arr, hk_as_range(val2));
  hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
  return HK_STATUS_ERROR;
}

static inline int slice_array(hk_vm_t *vm, hk_value_t *slots, hk_array_t *arr, hk_range_t *range)
{
  int arr_end = arr->length - 1;
  long start = range->start;
  long end = range->end;
  hk_array_t *result;
  if (start > end || start > arr_end || end < 0)
  {
    result = hk_array_new(0);
    goto end;
  }
  if (start <= 0 && end >= arr_end)
  {
    --vm->top;
    hk_range_release(range);
    return HK_STATUS_OK;
  }
  int length = end - start + 1;
  result = hk_array_allocate(length);
  result->length = length;
  for (int i = start, j = 0; i <= end ; ++i, ++j)
  {
    hk_value_t elem = arr->elements[i];
    hk_value_incr_ref(elem);
    result->elements[j] = elem;
  }
end:
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->top;
  hk_array_release(arr);
  hk_range_release(range);
  return HK_STATUS_OK;
}

static inline int fetch_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  if (!hk_is_int(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  int index = (int) val2.as.number;
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  hk_value_t elem = arr->elements[index];
  if (push(vm, elem) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_incr_ref(elem);
  return HK_STATUS_OK;
}

static inline void set_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 2];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  hk_value_t val3 = slots[2];
  hk_array_t *arr = hk_as_array(val1);
  int index = (int) val2.as.number;
  hk_array_t *result = hk_array_set_element(arr, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  vm->top -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
}

static inline int put_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 2];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  hk_value_t val3 = slots[2];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  if (!hk_is_int(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  int index = (int) val2.as.number;
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  hk_array_t *result = hk_array_set_element(arr, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  vm->top -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
  return HK_STATUS_OK;
}

static inline int delete_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  if (!hk_is_int(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  int index = (int) val2.as.number;
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  hk_array_t *result = hk_array_delete_element(arr, index);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->top;
  hk_array_release(arr);
  return HK_STATUS_OK;
}

static inline int inplace_add_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  if (arr->ref_count == 2)
  {
    hk_array_inplace_add_element(arr, val2);
    --vm->top;
    hk_value_decr_ref(val2);
    return HK_STATUS_OK;
  }
  hk_array_t *result = hk_array_add_element(arr, val2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->top;
  hk_array_release(arr);
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline int inplace_put_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 2];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  hk_value_t val3 = slots[2];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  if (!hk_is_int(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  int index = (int) val2.as.number;
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  if (arr->ref_count == 2)
  {
    hk_array_inplace_set_element(arr, index, val3);
    vm->top -= 2;
    hk_value_decr_ref(val3);
    return HK_STATUS_OK;
  }
  hk_array_t *result = hk_array_set_element(arr, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  vm->top -= 2;
  hk_array_release(arr);
  hk_value_decr_ref(val3);
  return HK_STATUS_OK;
}

static inline int inplace_delete_element(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_array(val1))
  {
    hk_runtime_error("type error: cannot use %s as an array", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  if (!hk_is_int(val2))
  {
    hk_runtime_error("type error: array cannot be indexed by %s", hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  hk_array_t *arr = hk_as_array(val1);
  int index = (int) val2.as.number;
  if (index < 0 || index >= arr->length)
  {
    hk_runtime_error("range error: index %d is out of bounds for array of length %d",
      index, arr->length);
    return HK_STATUS_ERROR;
  }
  if (arr->ref_count == 2)
  {
    hk_array_inplace_delete_element(arr, index);
    --vm->top;
    return HK_STATUS_OK;
  }
  hk_array_t *result = hk_array_delete_element(arr, index);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->top;
  hk_array_release(arr);
  return HK_STATUS_OK;
}

static inline int get_field(hk_vm_t *vm, hk_string_t *name)
{
  hk_value_t *slots = &vm->slots[vm->top];
  hk_value_t val = slots[0];
  if (!hk_is_instance(val))
  {
    hk_runtime_error("type error: cannot use %s as an instance",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  hk_instance_t *inst = hk_as_instance(val);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  hk_value_t value = inst->values[index];
  hk_value_incr_ref(value);
  slots[0] = value;
  hk_instance_release(inst);
  return HK_STATUS_OK;
}

static inline int fetch_field(hk_vm_t *vm, hk_string_t *name)
{
  hk_value_t *slots = &vm->slots[vm->top];
  hk_value_t val = slots[0];
  if (!hk_is_instance(val))
  {
    hk_runtime_error("type error: cannot use %s as an instance", hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  hk_instance_t *inst = hk_as_instance(val);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  if (push(vm, hk_number_value(index)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_t value = inst->values[index];
  if (push(vm, value) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_incr_ref(value);
  return HK_STATUS_OK;
}

static inline void set_field(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 2];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  hk_value_t val3 = slots[2];
  hk_instance_t *inst = hk_as_instance(val1);
  int index = (int) val2.as.number;
  hk_instance_t *result = hk_instance_set_field(inst, index, val3);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  vm->top -= 2;
  hk_instance_release(inst);
  hk_value_decr_ref(val3);
}

static inline int put_field(hk_vm_t *vm, hk_string_t *name)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_instance(val1))
  {
    hk_runtime_error("type error: cannot use %s as an instance", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  hk_instance_t *inst = hk_as_instance(val1);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  hk_instance_t *result = hk_instance_set_field(inst, index, val2);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  --vm->top;
  hk_instance_release(inst);
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline int inplace_put_field(hk_vm_t *vm, hk_string_t *name)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_instance(val1))
  {
    hk_runtime_error("type error: cannot use %s as an instance", hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  hk_instance_t *inst = hk_as_instance(val1);
  int index = hk_struct_index_of(inst->ztruct, name);
  if (index == -1)
  {
    hk_runtime_error("no field %.*s on struct", name->length, name->chars);
    return HK_STATUS_ERROR;
  }
  if (inst->ref_count == 2)
  {
    hk_instance_inplace_set_field(inst, index, val2);
    --vm->top;
    hk_value_decr_ref(val2);
    return HK_STATUS_OK;
  }
  hk_instance_t *result = hk_instance_set_field(inst, index, val2);
  hk_incr_ref(result);
  slots[0] = hk_instance_value(result);
  --vm->top;
  hk_instance_release(inst);
  hk_value_decr_ref(val2);
  return HK_STATUS_OK;
}

static inline void equal(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  slots[0] = hk_value_equal(val1, val2) ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --vm->top;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline int greater(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  int result;
  if (hk_value_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result > 0 ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --vm->top;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline int less(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  int result;
  if (hk_value_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result < 0 ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  --vm->top;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline void not_equal(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  slots[0] = hk_value_equal(val1, val2) ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --vm->top;
  hk_value_release(val1);
  hk_value_release(val2);
}

static inline int not_greater(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  int result;
  if (hk_value_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result > 0 ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --vm->top;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline int not_less(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  int result;
  if (hk_value_compare(val1, val2, &result) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  slots[0] = result < 0 ? HK_FALSE_VALUE : HK_TRUE_VALUE;
  --vm->top;
  hk_value_release(val1);
  hk_value_release(val2);
  return HK_STATUS_OK;
}

static inline int add(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (hk_is_number(val1))
  {
    if (!hk_is_number(val2))
    {
      hk_runtime_error("type error: cannot add %s to number", hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    double data = val1.as.number + val2.as.number;
    slots[0] = hk_number_value(data);
    --vm->top;
    return HK_STATUS_OK;
  }
  if (hk_is_string(val1))
  {
    if (!hk_is_string(val2))
    {
      hk_runtime_error("type error: cannot concatenate string and %s",
        hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    return concat_strings(vm, slots, val1, val2);
  }
  if (hk_is_array(val1))
  {
    if (!hk_is_array(val2))
    {
      hk_runtime_error("type error: cannot concatenate array and %s",
        hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    return concat_arrays(vm, slots, val1, val2);
  }
  hk_runtime_error("type error: cannot add %s to %s", hk_type_name(val2.type),
    hk_type_name(val1.type));
  return HK_STATUS_ERROR;
}

static inline int concat_strings(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2)
{
  hk_string_t *str1 = hk_as_string(val1);
  if (!str1->length)
  {
    slots[0] = val2;
    --vm->top;
    hk_string_release(str1);
    return HK_STATUS_OK;
  }
  hk_string_t *str2 = hk_as_string(val2);
  if (!str2->length)
  {
    --vm->top;
    hk_string_release(str2);
    return HK_STATUS_OK;
  }
  if (str1->ref_count == 1)
  {
    hk_string_inplace_concat(str1, str2);
    --vm->top;
    hk_string_release(str2);
    return HK_STATUS_OK;
  }
  hk_string_t *result = hk_string_concat(str1, str2);
  hk_incr_ref(result);
  slots[0] = hk_string_value(result);
  --vm->top;
  hk_string_release(str1);
  hk_string_release(str2);
  return HK_STATUS_OK;
}

static inline int concat_arrays(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2)
{
  hk_array_t *arr1 = hk_as_array(val1);
  if (!arr1->length)
  {
    slots[0] = val2;
    --vm->top;
    hk_array_release(arr1);
    return HK_STATUS_OK;
  }
  hk_array_t *arr2 = hk_as_array(val2);
  if (!arr2->length)
  {
    --vm->top;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  if (arr1->ref_count == 1)
  {
    hk_array_inplace_concat(arr1, arr2);
    --vm->top;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  hk_array_t *result = hk_array_concat(arr1, arr2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->top;
  hk_array_release(arr1);
  hk_array_release(arr2);
  return HK_STATUS_OK;
}

static inline int subtract(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (hk_is_number(val1))
  {
    if (!hk_is_number(val2))
    {
      hk_runtime_error("type error: cannot subtract %s from number",
        hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    double data = val1.as.number - val2.as.number;
    slots[0] = hk_number_value(data);
    --vm->top;
    return HK_STATUS_OK;
  }
  if (hk_is_array(val1))
  {
    if (!hk_is_array(val2))
    {
      hk_runtime_error("type error: cannot diff between array and %s",
        hk_type_name(val2.type));
      return HK_STATUS_ERROR;
    }
    return diff_arrays(vm, slots, val1, val2);
  }
  hk_runtime_error("type error: cannot subtract %s from %s", hk_type_name(val2.type),
    hk_type_name(val1.type));
  return HK_STATUS_ERROR;
}

static inline int diff_arrays(hk_vm_t *vm, hk_value_t *slots, hk_value_t val1, hk_value_t val2)
{
  hk_array_t *arr1 = hk_as_array(val1);
  hk_array_t *arr2 = hk_as_array(val2);
  if (!arr1->length || !arr2->length)
  {
    --vm->top;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  if (arr1->ref_count == 1)
  {
    hk_array_inplace_diff(arr1, arr2);
    --vm->top;
    hk_array_release(arr2);
    return HK_STATUS_OK;
  }
  hk_array_t *result = hk_array_diff(arr1, arr2);
  hk_incr_ref(result);
  slots[0] = hk_array_value(result);
  --vm->top;
  hk_array_release(arr1);
  hk_array_release(arr2);
  return HK_STATUS_OK;
}

static inline int multiply(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot multiply %s to %s", hk_type_name(val2.type),
      hk_type_name(val1.type));
    return HK_STATUS_ERROR;
  }
  double data = val1.as.number * val2.as.number;
  slots[0] = hk_number_value(data);
  --vm->top;
  return HK_STATUS_OK;
}

static inline int divide(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot divide %s by %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  double data = val1.as.number / val2.as.number;
  slots[0] = hk_number_value(data);
  --vm->top;
  return HK_STATUS_OK;
}

static inline int modulo(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top - 1];
  hk_value_t val1 = slots[0];
  hk_value_t val2 = slots[1];
  if (!hk_is_number(val1) || !hk_is_number(val2))
  {
    hk_runtime_error("type error: cannot apply modulo operation between %s and %s",
      hk_type_name(val1.type), hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  double data = fmod(val1.as.number, val2.as.number);
  slots[0] = hk_number_value(data);
  --vm->top;
  return HK_STATUS_OK;
}

static inline int negate(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top];
  hk_value_t val = slots[0];
  if (!hk_is_number(val))
  {
    hk_runtime_error("type error: cannot apply unary minus operation to %s",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  double data = -val.as.number;
  slots[0] = hk_number_value(data);
  return HK_STATUS_OK;
}

static inline void not(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top];
  hk_value_t val = slots[0];
  slots[0] = hk_is_falsey(val) ? HK_TRUE_VALUE : HK_FALSE_VALUE;
  hk_value_release(val);
}

static inline int incr(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top];
  hk_value_t val = slots[0];
  if (!hk_is_number(val))
  {
    hk_runtime_error("type error: cannot increment value of type %s",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  ++slots[0].as.number;
  return HK_STATUS_OK;
}

static inline int decr(hk_vm_t *vm)
{
  hk_value_t *slots = &vm->slots[vm->top];
  hk_value_t val = slots[0];
  if (!hk_is_number(val))
  {
    hk_runtime_error("type error: cannot decrement value of type %s",
      hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  --slots[0].as.number;
  return HK_STATUS_OK;
}

static inline int call(hk_vm_t *vm, int num_args)
{
  hk_value_t *slots = &vm->slots[vm->top - num_args];
  hk_value_t val = slots[0];
  if (!hk_is_callable(val))
  {
    hk_runtime_error("type error: cannot call value of type %s",
      hk_type_name(val.type));
    discard_frame(vm, slots);
    return HK_STATUS_ERROR;
  }
  if (hk_is_native(val))
  {
    hk_native_t *native = hk_as_native(val);
    if (check_arity(native->arity, native->name, num_args) == HK_STATUS_ERROR)
    {
      discard_frame(vm, slots);
      return HK_STATUS_ERROR;
    }
    int status;
    if ((status = native->call(vm, slots)) != HK_STATUS_OK)
    {
      if (status != HK_STATUS_NO_TRACE)
        print_trace(native->name, NULL, 0);
      discard_frame(vm, slots);
      return HK_STATUS_ERROR;
    }
    hk_native_release(native);
    move_result(vm, slots);
    return HK_STATUS_OK;
  }
  hk_closure_t *cl = hk_as_closure(val);
  hk_function_t *fn = cl->fn;
  if (check_arity(fn->arity, fn->name, num_args) == HK_STATUS_ERROR)
  {
    discard_frame(vm, slots);
    return HK_STATUS_ERROR;
  }
  int line;
  if (call_function(vm, slots, cl, &line) == HK_STATUS_ERROR)
  {
    print_trace(fn->name, fn->file, line);
    discard_frame(vm, slots);
    return HK_STATUS_ERROR;
  }
  hk_closure_release(cl);
  move_result(vm, slots);
  return HK_STATUS_OK;
}

static inline int check_arity(int arity, hk_string_t *name, int num_args)
{
  if (num_args >= arity)
    return HK_STATUS_OK;
  const char *fmt = arity < 2 ? "%s() expects %d argument but got %d" :
    "%s() expects %d arguments but got %d";
  char *name_chars = name ? name->chars : "<anonymous>";
  hk_runtime_error(fmt, name_chars, arity, num_args);
  return HK_STATUS_ERROR;
}

static inline void print_trace(hk_string_t *name, hk_string_t *file, int line)
{
  char *name_chars = name ? name->chars : "<anonymous>";
  if (file)
  {
    fprintf(stderr, "  at %s() in %.*s:%d\n", name_chars, file->length, file->chars, line);
    return;
  }
  fprintf(stderr, "  at %s() in <native>\n", name_chars);
}

static inline int call_function(hk_vm_t *vm, hk_value_t *locals, hk_closure_t *cl, int *line)
{
  hk_value_t *slots = vm->slots;
  hk_function_t *fn = cl->fn;
  hk_value_t *nonlocals = cl->nonlocals;
  uint8_t *code = fn->chunk.bytes;
  hk_value_t *consts = fn->consts->elements;
  hk_function_t **functions = fn->functions;
  uint8_t *pc = code;
  for (;;)
  {
    int op = read_byte(&pc);
    switch (op)
    {
    case HK_OP_NIL:
      if (push(vm, HK_NIL_VALUE) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_FALSE:
      if (push(vm, HK_FALSE_VALUE) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_TRUE:
      if (push(vm, HK_TRUE_VALUE) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INT:
      if (push(vm, hk_number_value(read_word(&pc))) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CONSTANT:
      {
        hk_value_t val = consts[read_byte(&pc)];
        if (push(vm, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_RANGE:
      if (range(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_ARRAY:
      if (array(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_STRUCT:
      if (ztruct(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INSTANCE:
      if (instance(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CONSTRUCT:
      if (construct(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CLOSURE:
      if (closure(vm, functions[read_byte(&pc)]) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_UNPACK:
      if (unpack(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_DESTRUCT:
      if (destruct(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_POP:
      hk_value_release(slots[vm->top--]);
      break;
    case HK_OP_GLOBAL:
      {
        hk_value_t val = slots[read_byte(&pc)];
        if (push(vm, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_NONLOCAL:
      {
        hk_value_t val = nonlocals[read_byte(&pc)];
        if (push(vm, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_GET_LOCAL:
      {
        hk_value_t val = locals[read_byte(&pc)];
        if (push(vm, val) == HK_STATUS_ERROR)
          goto error;
        hk_value_incr_ref(val);
      }
      break;
    case HK_OP_SET_LOCAL:
      {
        int index = read_byte(&pc);
        hk_value_t val = slots[vm->top];
        --vm->top;
        hk_value_release(locals[index]);
        locals[index] = val;
      }
      break;
    case HK_OP_ADD_ELEMENT:
      if (add_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_GET_ELEMENT:
      if (get_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_FETCH_ELEMENT:
      if (fetch_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_SET_ELEMENT:
      set_element(vm);
      break;
    case HK_OP_PUT_ELEMENT:
      if (put_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_DELETE_ELEMENT:
      if (delete_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_ADD_ELEMENT:
      if (inplace_add_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_PUT_ELEMENT:
      if (inplace_put_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_DELETE_ELEMENT:
      if (inplace_delete_element(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_GET_FIELD:
      if (get_field(vm, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_FETCH_FIELD:
      if (fetch_field(vm, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_SET_FIELD:
      set_field(vm);
      break;
    case HK_OP_PUT_FIELD:
      if (put_field(vm, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_INPLACE_PUT_FIELD:
      if (inplace_put_field(vm, hk_as_string(consts[read_byte(&pc)])) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_JUMP:
      pc = &code[read_word(&pc)];
      break;
    case HK_OP_JUMP_IF_FALSE:
      {
        int offset = read_word(&pc);
        hk_value_t val = slots[vm->top];
        if (hk_is_falsey(val))
          pc = &code[offset];
        hk_value_release(val);
        --vm->top;
      }
      break;
    case HK_OP_OR:
      {
        int offset = read_word(&pc);
        hk_value_t val = slots[vm->top];
        if (hk_is_truthy(val))
        {
          pc = &code[offset];
          break;
        }
        hk_value_release(val);
        --vm->top;
      }
      break;
    case HK_OP_AND:
      {
        int offset = read_word(&pc);
        hk_value_t val = slots[vm->top];
        if (hk_is_falsey(val))
        {
          pc = &code[offset];
          break;
        }
        hk_value_release(val);
        --vm->top;
      }
      break;
    case HK_OP_MATCH:
      {
        int offset = read_word(&pc);
        hk_value_t val1 = slots[vm->top - 1];
        hk_value_t val2 = slots[vm->top];
        if (hk_value_equal(val1, val2))
        {
          hk_value_release(val1);
          hk_value_release(val2);
          vm->top -= 2;
          break;
        }
        pc = &code[offset];
        hk_value_release(val2);
        --vm->top;
      }
      break;
    case HK_OP_EQUAL:
      equal(vm);
      break;
    case HK_OP_GREATER:
      if (greater(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_LESS:
      if (less(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NOT_EQUAL:
      not_equal(vm);
      break;
    case HK_OP_NOT_GREATER:
      if (not_greater(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NOT_LESS:
      if (not_less(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_ADD:
      if (add(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_SUBTRACT:
      if (subtract(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_MULTIPLY:
      if (multiply(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_DIVIDE:
      if (divide(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_MODULO:
      if (modulo(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NEGATE:
      if (negate(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_NOT:
      not(vm);
      break;
    case HK_OP_INCR:
      if (incr(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_DECR:
      if (decr(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_CALL:
      if (call(vm, read_byte(&pc)) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_LOAD_MODULE:
      if (load_module(vm) == HK_STATUS_ERROR)
        goto error;
      break;
    case HK_OP_RETURN:
      return HK_STATUS_OK;
    case HK_OP_RETURN_NIL:
      if (push(vm, HK_NIL_VALUE) == HK_STATUS_ERROR)
        goto error;
      return HK_STATUS_OK;
    }
  }
error:
  *line = hk_function_get_line(fn, (int) (pc - fn->chunk.bytes));
  return HK_STATUS_ERROR;
}

static inline void discard_frame(hk_vm_t *vm, hk_value_t *slots)
{
  while (&vm->slots[vm->top] >= slots)
    hk_value_release(vm->slots[vm->top--]);
}

static inline void move_result(hk_vm_t *vm, hk_value_t *slots)
{
  slots[0] = vm->slots[vm->top];
  --vm->top;
  while (&vm->slots[vm->top] > slots)
    hk_value_release(vm->slots[vm->top--]);
}

void hk_vm_init(hk_vm_t *vm, int min_capacity)
{
  int capacity = hk_nearest_power_of_two(HK_VM_MIN_CAPACITY, min_capacity);
  vm->limit = capacity - 1;
  vm->top = -1;
  vm->slots = (hk_value_t *) hk_allocate(sizeof(*vm->slots) * capacity);
  load_globals(vm);
  init_module_cache();
}

void hk_vm_free(hk_vm_t *vm)
{
  free_module_cache();
  hk_assert(vm->top == num_globals() - 1, "stack must contain the globals");
  while (vm->top > -1)
    hk_value_release(vm->slots[vm->top--]);
  free(vm->slots);
}

int hk_vm_push(hk_vm_t *vm, hk_value_t val)
{
  if (push(vm, val) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_value_incr_ref(val);
  return HK_STATUS_OK;
}

int hk_vm_push_nil(hk_vm_t *vm)
{
  return push(vm, HK_NIL_VALUE);
}

int hk_vm_push_boolean(hk_vm_t *vm, bool data)
{
  return push(vm, data ? HK_TRUE_VALUE : HK_FALSE_VALUE);
}

int hk_vm_push_number(hk_vm_t *vm, double data)
{
  return push(vm, hk_number_value(data));
}

int hk_vm_push_string(hk_vm_t *vm, hk_string_t *str)
{
  if (push(vm, hk_string_value(str)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(str);
  return HK_STATUS_OK;
}

int hk_vm_push_string_from_chars(hk_vm_t *vm, int length, const char *chars)
{   
  hk_string_t *str = hk_string_from_chars(length, chars);
  if (hk_vm_push_string(vm, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_vm_push_string_from_stream(hk_vm_t *vm, FILE *stream, const char terminal)
{
  hk_string_t *str = hk_string_from_stream(stream, terminal);
  if (hk_vm_push_string(vm, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_vm_push_range(hk_vm_t *vm, hk_range_t *range)
{
  if (push(vm, hk_range_value(range)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(range);
  return HK_STATUS_OK;
}

int hk_vm_push_array(hk_vm_t *vm, hk_array_t *arr)
{
  if (push(vm, hk_array_value(arr)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(arr);
  return HK_STATUS_OK;
}

int hk_vm_push_struct(hk_vm_t *vm, hk_struct_t *ztruct)
{
  if (push(vm, hk_struct_value(ztruct)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(ztruct);
  return HK_STATUS_OK;
}

int hk_vm_push_instance(hk_vm_t *vm, hk_instance_t *inst)
{
  if (push(vm, hk_instance_value(inst)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(inst);
  return HK_STATUS_OK;
}

int hk_vm_push_iterator(hk_vm_t *vm, hk_iterator_t *it)
{
  if (push(vm, hk_iterator_value(it)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(it);
  return HK_STATUS_OK;
}

int hk_vm_push_closure(hk_vm_t *vm, hk_closure_t *cl)
{
  if (push(vm, hk_closure_value(cl)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(cl);
  return HK_STATUS_OK;
}

int hk_vm_push_native(hk_vm_t *vm, hk_native_t *native)
{
  if (push(vm, hk_native_value(native)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(native);
  return HK_STATUS_OK;
}

int hk_vm_push_new_native(hk_vm_t *vm, const char *name, int arity, int (*call)(hk_vm_t *, hk_value_t *))
{
  hk_native_t *native = hk_native_new(hk_string_from_chars(-1, name), arity, call);
  if (hk_vm_push_native(vm, native) == HK_STATUS_ERROR)
  {
    hk_native_free(native);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_vm_push_userdata(hk_vm_t *vm, hk_userdata_t *udata)
{
  if (push(vm, hk_userdata_value(udata)) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_incr_ref(udata);
  return HK_STATUS_OK;
}

int hk_vm_array(hk_vm_t *vm, int length)
{
  return array(vm, length);
}

int hk_vm_struct(hk_vm_t *vm, int length)
{
  return ztruct(vm, length);
}

int hk_vm_instance(hk_vm_t *vm, int length)
{
  return instance(vm, length);
}

int hk_vm_construct(hk_vm_t *vm, int length)
{
  return construct(vm, length);
}

void hk_vm_pop(hk_vm_t *vm)
{
  hk_assert(vm->top > -1, "stack underflow");
  hk_value_t val = vm->slots[vm->top];
  --vm->top;
  hk_value_release(val);
}

int hk_vm_call(hk_vm_t *vm, int num_args)
{
  return call(vm, num_args);
}

int hk_vm_check_type(hk_value_t *args, int index, int type)
{
  int val_type = args[index].type;
  if (val_type != type)
  {
    hk_runtime_error("type error: argument #%d must be of the type %s, %s given", index,
      hk_type_name(type), hk_type_name(val_type));
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_vm_check_types(hk_value_t *args, int index, int num_types, int types[])
{
  int val_type = args[index].type;
  bool match = false;
  for (int i = 0; i < num_types; ++i)
    if ((match = val_type == types[i]))
      break;
  if (!match)
  {
    type_error(index, num_types, types, val_type);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_vm_check_boolean(hk_value_t *args, int index)
{
  return hk_vm_check_type(args, index, HK_TYPE_BOOLEAN);
}

int hk_vm_check_number(hk_value_t *args, int index)
{
  return hk_vm_check_type(args, index, HK_TYPE_NUMBER);
}

int hk_vm_check_integer(hk_value_t *args, int index)
{
  hk_value_t val = args[index];
  if (!hk_is_integer(val))
  {
    hk_runtime_error("type error: argument #%d must be of the type integer, %s given",
      index, hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_vm_check_int(hk_value_t *args, int index)
{
  hk_value_t val = args[index];
  if (!hk_is_int(val))
  {
    hk_runtime_error("type error: argument #%d must be of the type int, %s given",
      index, hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_vm_check_string(hk_value_t *args, int index)
{
  return hk_vm_check_type(args, index, HK_TYPE_STRING);
}

int hk_vm_check_range(hk_value_t *args, int index)
{
  return hk_vm_check_type(args, index, HK_TYPE_RANGE);
}

int hk_vm_check_array(hk_value_t *args, int index)
{
  return hk_vm_check_type(args, index, HK_TYPE_ARRAY);
}

int hk_vm_check_struct(hk_value_t *args, int index)
{
  return hk_vm_check_type(args, index, HK_TYPE_STRUCT);
}

int hk_vm_check_instance(hk_value_t *args, int index)
{
  return hk_vm_check_type(args, index, HK_TYPE_INSTANCE);
}

int hk_vm_check_iterator(hk_value_t *args, int index)
{
  return hk_vm_check_type(args, index, HK_TYPE_ITERATOR);
}

int hk_vm_check_callable(hk_value_t *args, int index)
{
  return hk_vm_check_type(args, index, HK_TYPE_CALLABLE);
}

int hk_vm_check_userdata(hk_value_t *args, int index)
{
  return hk_vm_check_type(args, index, HK_TYPE_USERDATA);
}
