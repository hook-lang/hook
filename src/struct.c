//
// struct.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include <hook/struct.h>
#include <string.h>
#include <hook/memory.h>
#include <hook/string.h>

static inline HkField **allocate_table(int capacity);
static inline HkField *add_field(HkStruct *ztruct, HkString *name);
static inline void grow(HkStruct *ztruct);

static inline HkField **allocate_table(int capacity)
{
  HkField **table = (HkField **) hk_allocate(sizeof(*table) * capacity);
  for (int i = 0; i < capacity; ++i)
    table[i] = NULL;
  return table;
}

static inline HkField *add_field(HkStruct *ztruct, HkString *name)
{
  HkField *field = &ztruct->fields[ztruct->length];
  hk_incr_ref(name);
  field->name = name;
  field->index = ztruct->length;
  ++ztruct->length;
  return field;
}

static inline void grow(HkStruct *ztruct)
{
  int length = ztruct->length;
  if (length / HK_STRUCT_MAX_LOAD_FACTOR <= ztruct->capacity)
    return;
  int capacity = ztruct->capacity << 1;
  ztruct->capacity = capacity;
  int mask = capacity - 1;
  ztruct->mask = mask;
  ztruct->fields = (HkField *) hk_reallocate(ztruct->fields,
    sizeof(*ztruct->fields) * capacity);
  HkField **table = allocate_table(capacity);
  hk_free(ztruct->table);
  ztruct->table = table;
  HkField *fields = ztruct->fields;
  for (int i = 0; i < length; i++)
  {
    HkField *field = &fields[i];
    int j = field->name->hash & mask;
    while (table[j])
      j = (j + 1) & mask;
    table[j] = field;
  }
}

HkStruct *hk_struct_new(HkString *name)
{
  int capacity = HK_STRUCT_MIN_CAPACITY;
  HkStruct *ztruct = (HkStruct *) hk_allocate(sizeof(*ztruct));
  ztruct->refCount = 0;
  ztruct->capacity = capacity;
  ztruct->mask = capacity - 1;
  ztruct->length = 0;
  if (name)
    hk_incr_ref(name);
  ztruct->name = name;
  ztruct->fields = (HkField *) hk_allocate(sizeof(*ztruct->fields) * capacity);
  ztruct->table = allocate_table(capacity);
  return ztruct;
}

void hk_struct_free(HkStruct *ztruct)
{
  HkString *name = ztruct->name;
  if (name)
    hk_string_release(name);
  HkField *fields = ztruct->fields;
  for (int i = 0; i < ztruct->length; ++i)
    hk_string_release(fields[i].name);
  hk_free(ztruct->fields);
  hk_free(ztruct->table);
  hk_free(ztruct);
}

void hk_struct_release(HkStruct *ztruct)
{
  hk_decr_ref(ztruct);
  if (hk_is_unreachable(ztruct))
    hk_struct_free(ztruct);
}

int hk_struct_index_of(HkStruct *ztruct, HkString *name)
{
  int mask = ztruct->mask;
  HkField **table = ztruct->table;
  int i = hk_string_hash(name) & mask;
  for (;;)
  {
    HkField *field = table[i];
    if (!field)
      break;
    if (hk_string_equal(name, field->name))
      return field->index;
    i = (i + 1) & mask;
  }
  return -1;
}

bool hk_struct_define_field(HkStruct *ztruct, HkString *name)
{
  int mask = ztruct->mask;
  HkField **table = ztruct->table;
  uint32_t h = hk_string_hash(name);
  int i = h & mask;
  for (;;)
  {
    HkField *field = table[i];
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

bool hk_struct_equal(HkStruct *ztruct1, HkStruct *ztruct2)
{
  if (ztruct1 == ztruct2)
    return true;
  if (ztruct1->length != ztruct2->length)
    return false;
  for (int i = 0; i < ztruct1->length; ++i)
    if (!hk_string_equal(ztruct1->fields[i].name, ztruct2->fields[i].name))
      return false;
  return true;
}

HkInstance *hk_instance_new(HkStruct *ztruct)
{
  int size = sizeof(HkInstance) + sizeof(HkValue) * (ztruct->length - 1);
  HkInstance *inst = (HkInstance *) hk_allocate(size);
  inst->refCount = 0;
  hk_incr_ref(ztruct);
  inst->ztruct = ztruct;
  return inst;
}

void hk_instance_free(HkInstance *inst)
{
  HkStruct *ztruct = inst->ztruct;
  int length = ztruct->length;
  hk_struct_release(ztruct);
  for (int i = 0; i < length; ++i)
    hk_value_release(inst->values[i]);
  hk_free(inst);
}

void hk_instance_release(HkInstance *inst)
{
  hk_decr_ref(inst);
  if (hk_is_unreachable(inst))
    hk_instance_free(inst);
}

HkInstance *hk_instance_set_field(HkInstance *inst, int index, HkValue value)
{
  HkStruct *ztruct = inst->ztruct;
  HkInstance *result = hk_instance_new(ztruct);
  for (int i = 0; i < index; ++i)
  {
    HkValue val = inst->values[i];
    hk_value_incr_ref(val);
    result->values[i] = val;
  }
  hk_value_incr_ref(value);
  result->values[index] = value;
  for (int i = index + 1; i < ztruct->length; ++i)
  {
    HkValue val = inst->values[i];
    hk_value_incr_ref(val);
    result->values[i] = val;
  }
  return result;
}

void hk_instance_inplace_set_field(HkInstance *inst, int index, HkValue value)
{
  hk_value_incr_ref(value);
  hk_value_release(inst->values[index]);
  inst->values[index] = value;
}

void hk_instance_print(HkInstance *inst)
{
  printf("{");
  int length = inst->ztruct->length;
  if (!length)
  {
    printf("}");
    return;
  }
  HkField *fields = inst->ztruct->fields;
  HkValue *values = inst->values;
  HkField *field = &fields[0];
  HkString *name = field->name;
  hk_string_print(name, false);
  printf(": ");
  hk_value_print(values[field->index], true);
  for (int i = 1; i < length; ++i)
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

bool hk_instance_equal(HkInstance *inst1, HkInstance *inst2)
{
  if (inst1 == inst2)
    return true;
  if (!hk_struct_equal(inst1->ztruct, inst2->ztruct))
    return false;
  int length = inst1->ztruct->length;
  for (int i = 0; i < length; ++i)
    if (!hk_value_equal(inst1->values[i], inst2->values[i]))
      return false;  
  return true;
}
