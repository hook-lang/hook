//
// Hook Programming Language
// hook_struct.h
//

#ifndef HOOK_STRUCT_H
#define HOOK_STRUCT_H

#include "hook_string.h"

#define STRUCT_MIN_CAPACITY    (1 << 3)
#define STRUCT_MAX_LOAD_FACTOR 0.75

typedef struct
{
  hk_string_t *name;
  int index;
} hk_field_t;

typedef struct
{
  HK_OBJECT_HEADER
  int capacity;
  int mask;
  int length;
  hk_string_t *name;
  hk_field_t *fields;
  hk_field_t **table;
} hk_struct_t;

typedef struct
{
  HK_OBJECT_HEADER
  hk_struct_t *ztruct;
  hk_value_t values[0];
} hk_instance_t;

hk_struct_t *hk_struct_new(hk_string_t *name);
void hk_struct_free(hk_struct_t *ztruct);
void hk_struct_release(hk_struct_t *ztruct);
int hk_struct_index_of(hk_struct_t *ztruct, hk_string_t *name);
bool hk_struct_define_field(hk_struct_t *ztruct, hk_string_t *name);
bool hk_struct_equal(hk_struct_t *ztruct1, hk_struct_t *ztruct2);
hk_instance_t *hk_instance_allocate(hk_struct_t *ztruct);
void hk_instance_free(hk_instance_t *inst);
void hk_instance_release(hk_instance_t *inst);
hk_instance_t *hk_instance_set_field(hk_instance_t *inst, int index, hk_value_t value);
void hk_instance_inplace_set_field(hk_instance_t *inst, int index, hk_value_t value);
void hk_instance_print(hk_instance_t *inst);
bool hk_instance_equal(hk_instance_t *inst1, hk_instance_t *inst2);

#endif // HOOK_STRUCT_H
