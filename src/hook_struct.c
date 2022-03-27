//
// Hook Programming Language
// hook_struct.c
//

#include "hook_struct.h"
#include <stdlib.h>
#include <string.h>
#include "hook_hash.h"
#include "hook_string.h"
#include "hook_memory.h"

static inline hk_field_t **allocate_table(int32_t capacity);
static inline hk_field_t *add_field(hk_struct_t *ztruct, hk_string_t *name);
static inline void grow(hk_struct_t *ztruct);

static inline hk_field_t **allocate_table(int32_t capacity)
{
  hk_field_t **table = (hk_field_t **) hk_allocate(sizeof(*table) * capacity);
  for (int32_t i = 0; i < capacity; ++i)
    table[i] = NULL;
  return table;
}

static inline hk_field_t *add_field(hk_struct_t *ztruct, hk_string_t *name)
{
  hk_field_t *field = &ztruct->fields[ztruct->length];
  hk_incr_ref(name);
  field->name = name;
  field->index = ztruct->length;
  ++ztruct->length;
  return field;
}

static inline void grow(hk_struct_t *ztruct)
{
  int32_t length = ztruct->length;
  if (length / STRUCT_MAX_LOAD_FACTOR <= ztruct->capacity)
    return;
  int32_t capacity = ztruct->capacity << 1;
  ztruct->capacity = capacity;
  int32_t mask = capacity - 1;
  ztruct->mask = mask;
  ztruct->fields = (hk_field_t *) hk_reallocate(ztruct->fields,
    sizeof(*ztruct->fields) * capacity);
  hk_field_t **table = allocate_table(capacity);
  free(ztruct->table);
  ztruct->table = table;
  hk_field_t *fields = ztruct->fields;
  for (int32_t i = 0; i < length; i++)
  {
    hk_field_t *field = &fields[i];
    int32_t j = field->name->hash & mask;
    while (table[j])
      j = (j + 1) & mask;
    table[j] = field;
  }
}

hk_struct_t *hk_struct_new(hk_string_t *name)
{
  int32_t capacity = STRUCT_MIN_CAPACITY;
  hk_struct_t *ztruct = (hk_struct_t *) hk_allocate(sizeof(*ztruct));
  ztruct->ref_count = 0;
  ztruct->capacity = capacity;
  ztruct->mask = capacity - 1;
  ztruct->length = 0;
  if (name)
    hk_incr_ref(name);
  ztruct->name = name;
  ztruct->fields = (hk_field_t *) hk_allocate(sizeof(*ztruct->fields) * capacity);
  ztruct->table = allocate_table(capacity);
  return ztruct;
}

void hk_struct_free(hk_struct_t *ztruct)
{
  hk_string_t *name = ztruct->name;
  if (name)
    hk_string_release(name);
  hk_field_t *fields = ztruct->fields;
  for (int32_t i = 0; i < ztruct->length; ++i)
    hk_string_release(fields[i].name);
  free(ztruct->fields);
  free(ztruct->table);
  free(ztruct);
}

void hk_struct_release(hk_struct_t *ztruct)
{
  hk_decr_ref(ztruct);
  if (hk_is_unreachable(ztruct))
    hk_struct_free(ztruct);
}

int32_t hk_struct_index_of(hk_struct_t *ztruct, hk_string_t *name)
{
  int32_t mask = ztruct->mask;
  hk_field_t **table = ztruct->table;
  int32_t i = hk_string_hash(name) & mask;
  for (;;)
  {
    hk_field_t *field = table[i];
    if (!field)
      break;
    if (hk_string_equal(name, field->name))
      return field->index;
    i = (i + 1) & mask;
  }
  return -1;
}

bool hk_struct_define_field(hk_struct_t *ztruct, hk_string_t *name)
{
  int32_t mask = ztruct->mask;
  hk_field_t **table = ztruct->table;
  uint32_t h = hk_string_hash(name);
  int32_t i = h & mask;
  for (;;)
  {
    hk_field_t *field = table[i];
    if (!field)
    {
      table[i] = add_field(ztruct, name);
      grow(ztruct);
      return true;
    }
    if (hk_string_equal(name, field->name))
      break;
    i = (i + 1) & mask;
  }
  return false;
}

bool hk_struct_equal(hk_struct_t *ztruct1, hk_struct_t *ztruct2)
{
  if (ztruct1 == ztruct2)
    return true;
  if (ztruct1->length != ztruct2->length)
    return false;
  for (int32_t i = 0; i < ztruct1->length; ++i)
    if (!hk_string_equal(ztruct1->fields[i].name, ztruct2->fields[i].name))
      return false;
  return true;
}

hk_instance_t *hk_instance_new(hk_struct_t *ztruct)
{
  int32_t size = sizeof(hk_instance_t) + sizeof(hk_value_t) * ztruct->length;
  hk_instance_t *inst = (hk_instance_t *) hk_allocate(size);
  inst->ref_count = 0;
  hk_incr_ref(ztruct);
  inst->ztruct = ztruct;
  return inst;
}

void hk_instance_free(hk_instance_t *inst)
{
  hk_struct_t *ztruct = inst->ztruct;
  int32_t length = ztruct->length;
  hk_struct_release(ztruct);
  for (int32_t i = 0; i < length; ++i)
    hk_value_release(inst->values[i]);
  free(inst);
}

void hk_instance_release(hk_instance_t *inst)
{
  hk_decr_ref(inst);
  if (hk_is_unreachable(inst))
    hk_instance_free(inst);
}

hk_instance_t *hk_instance_set_field(hk_instance_t *inst, int32_t index, hk_value_t value)
{
  hk_struct_t *ztruct = inst->ztruct;
  hk_instance_t *result = hk_instance_new(ztruct);
  for (int32_t i = 0; i < index; ++i)
  {
    hk_value_t val = inst->values[i];
    hk_value_incr_ref(val);
    result->values[i] = val;
  }
  hk_value_incr_ref(value);
  result->values[index] = value;
  for (int32_t i = index + 1; i < ztruct->length; ++i)
  {
    hk_value_t val = inst->values[i];
    hk_value_incr_ref(val);
    result->values[i] = val;
  }
  return result;
}

void hk_instance_inplace_set_field(hk_instance_t *inst, int32_t index, hk_value_t value)
{
  hk_value_incr_ref(value);
  hk_value_release(inst->values[index]);
  inst->values[index] = value;
}

void hk_instance_print(hk_instance_t *inst)
{
  printf("{");
  int32_t length = inst->ztruct->length;
  if (!length)
  {
    printf("}");
    return;
  }
  hk_field_t *fields = inst->ztruct->fields;
  hk_value_t *values = inst->values;
  hk_field_t *field = &fields[0];
  hk_string_t *name = field->name;
  hk_string_print(name, false);
  printf(": ");
  hk_value_print(values[field->index], true);
  for (int32_t i = 1; i < length; ++i)
  {
    field = &fields[i];
    name = field->name;
    printf(", ");
    hk_string_print(name, false);
    printf(": ");
    hk_value_print(values[field->index], true);
  }
  printf("}");
}

bool hk_instance_equal(hk_instance_t *inst1, hk_instance_t *inst2)
{
  if (inst1 == inst2)
    return true;
  if (!hk_struct_equal(inst1->ztruct, inst2->ztruct))
    return false;
  int32_t length = inst1->ztruct->length;
  for (int32_t i = 0; i < length; ++i)
    if (!hk_value_equal(inst1->values[i], inst2->values[i]))
      return false;  
  return true;
}
