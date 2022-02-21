//
// Hook Programming Language
// h_struct.h
//

#ifndef H_STRUCT_H
#define H_STRUCT_H

#include "h_string.h"

#define STRUCT_MIN_CAPACITY    (1 << 3)
#define STRUCT_MAX_LOAD_FACTOR 0.75

typedef struct
{
  string_t *name;
  int index;
} field_t;

typedef struct
{
  OBJECT_HEADER
  int capacity;
  int mask;
  int length;
  string_t *name;
  field_t *fields;
  field_t **table;
} struct_t;

typedef struct
{
  OBJECT_HEADER
  struct_t *ztruct;
  value_t values[0];
} instance_t;

struct_t *struct_new(string_t *name);
void struct_free(struct_t *ztruct);
void struct_release(struct_t *ztruct);
int struct_index_of(struct_t *ztruct, string_t *name);
bool struct_define_field(struct_t *ztruct, string_t *name);
bool struct_equal(struct_t *ztruct1, struct_t *ztruct2);
instance_t *instance_allocate(struct_t *ztruct);
void instance_free(instance_t *inst);
void instance_release(instance_t *inst);
instance_t *instance_set_field(instance_t *inst, int index, value_t value);
void instance_inplace_set_field(instance_t *inst, int index, value_t value);
void instance_print(instance_t *inst);
bool instance_equal(instance_t *inst1, instance_t *inst2);

#endif // H_STRUCT_H
