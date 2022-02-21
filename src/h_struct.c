//
// Hook Programming Language
// h_struct.c
//

#include "h_struct.h"
#include <stdlib.h>
#include <string.h>
#include "h_hash.h"
#include "h_string.h"
#include "h_memory.h"

static inline field_t **allocate_table(int capacity);
static inline field_t *add_field(struct_t *ztruct, string_t *name);
static inline void resize(struct_t *ztruct);

static inline field_t **allocate_table(int capacity)
{
  field_t **table = (field_t **) allocate(sizeof(*table) * capacity);
  for (int i = 0; i < capacity; ++i)
    table[i] = NULL;
  return table;
}

static inline field_t *add_field(struct_t *ztruct, string_t *name)
{
  field_t *field = &ztruct->fields[ztruct->length];
  INCR_REF(name);
  field->name = name;
  field->index = ztruct->length;
  ++ztruct->length;
  return field;
}

static inline void resize(struct_t *ztruct)
{
  int length = ztruct->length;
  if (length / STRUCT_MAX_LOAD_FACTOR <= ztruct->capacity)
    return;
  int capacity = ztruct->capacity << 1;
  ztruct->capacity = capacity;
  int mask = capacity - 1;
  ztruct->mask = mask;
  ztruct->fields = (field_t *) reallocate(ztruct->fields,
    sizeof(*ztruct->fields) * capacity);
  field_t **table = allocate_table(capacity);
  free(ztruct->table);
  ztruct->table = table;
  field_t *fields = ztruct->fields;
  for (int i = 0; i < length; i++)
  {
    field_t *field = &fields[i];
    int j = field->name->hash & mask;
    while (table[j])
      j = (j + 1) & mask;
    table[j] = field;
  }
}

struct_t *struct_new(string_t *name)
{
  int capacity = STRUCT_MIN_CAPACITY;
  struct_t *ztruct = (struct_t *) allocate(sizeof(*ztruct));
  ztruct->ref_count = 0;
  ztruct->capacity = capacity;
  ztruct->mask = capacity - 1;
  ztruct->length = 0;
  if (name)
    INCR_REF(name);
  ztruct->name = name;
  ztruct->fields = (field_t *) allocate(sizeof(*ztruct->fields) * capacity);
  ztruct->table = allocate_table(capacity);
  return ztruct;
}

void struct_free(struct_t *ztruct)
{
  string_t *name = ztruct->name;
  if (name)
    string_release(name);
  field_t *fields = ztruct->fields;
  for (int i = 0; i < ztruct->length; ++i)
    string_release(fields[i].name);
  free(ztruct->fields);
  free(ztruct->table);
  free(ztruct);
}

void struct_release(struct_t *ztruct)
{
  DECR_REF(ztruct);
  if (IS_UNREACHABLE(ztruct))
    struct_free(ztruct);
}

int struct_index_of(struct_t *ztruct, string_t *name)
{
  int mask = ztruct->mask;
  field_t **table = ztruct->table;
  int i = string_hash(name) & mask;
  for (;;)
  {
    field_t *field = table[i];
    if (!field)
      break;
    if (string_equal(name, field->name))
      return field->index;
    i = (i + 1) & mask;
  }
  return -1;
}

bool struct_define_field(struct_t *ztruct, string_t *name)
{
  int mask = ztruct->mask;
  field_t **table = ztruct->table;
  uint32_t h = string_hash(name);
  int i = h & mask;
  for (;;)
  {
    field_t *field = table[i];
    if (!field)
    {
      table[i] = add_field(ztruct, name);
      resize(ztruct);
      return true;
    }
    if (string_equal(name, field->name))
      break;
    i = (i + 1) & mask;
  }
  return false;
}

bool struct_equal(struct_t *ztruct1, struct_t *ztruct2)
{
  if (ztruct1 == ztruct2)
    return true;
  if (ztruct1->length != ztruct2->length)
    return false;
  for (int i = 0; i < ztruct1->length; ++i)
    if (!string_equal(ztruct1->fields[i].name, ztruct2->fields[i].name))
      return false;
  return true;
}

instance_t *instance_allocate(struct_t *ztruct)
{
  int size = sizeof(instance_t) + sizeof(value_t) * ztruct->length;
  instance_t *inst = (instance_t *) allocate(size);
  inst->ref_count = 0;
  INCR_REF(ztruct);
  inst->ztruct = ztruct;
  return inst;
}

void instance_free(instance_t *inst)
{
  struct_t *ztruct = inst->ztruct;
  int length = ztruct->length;
  struct_release(ztruct);
  for (int i = 0; i < length; ++i)
    value_release(inst->values[i]);
  free(inst);
}

void instance_release(instance_t *inst)
{
  DECR_REF(inst);
  if (IS_UNREACHABLE(inst))
    instance_free(inst);
}

instance_t *instance_set_field(instance_t *inst, int index, value_t value)
{
  struct_t *ztruct = inst->ztruct;
  instance_t *result = instance_allocate(ztruct);
  for (int i = 0; i < index; ++i)
  {
    value_t val = inst->values[i];
    VALUE_INCR_REF(val);
    result->values[i] = val;
  }
  VALUE_INCR_REF(value);
  result->values[index] = value;
  for (int i = index + 1; i < ztruct->length; ++i)
  {
    value_t val = inst->values[i];
    VALUE_INCR_REF(val);
    result->values[i] = val;
  }
  return result;
}

void instance_inplace_set_field(instance_t *inst, int index, value_t value)
{
  VALUE_INCR_REF(value);
  value_release(inst->values[index]);
  inst->values[index] = value;
}

void instance_print(instance_t *inst)
{
  printf("{");
  int length = inst->ztruct->length;
  if (!length)
  {
    printf("}");
    return;
  }
  field_t *fields = inst->ztruct->fields;
  value_t *values = inst->values;
  field_t *field = &fields[0];
  string_t *name = field->name;
  string_print(name, false);
  printf(": ");
  value_print(values[field->index], true);
  for (int i = 1; i < length; ++i)
  {
    field = &fields[i];
    name = field->name;
    printf(", ");
    string_print(name, false);
    printf(": ");
    value_print(values[field->index], true);
  }
  printf("}");
}

bool instance_equal(instance_t *inst1, instance_t *inst2)
{
  if (inst1 == inst2)
    return true;
  if (!struct_equal(inst1->ztruct, inst2->ztruct))
    return false;
  int length = inst1->ztruct->length;
  for (int i = 0; i < length; ++i)
    if (!value_equal(inst1->values[i], inst2->values[i]))
      return false;  
  return true;
}
