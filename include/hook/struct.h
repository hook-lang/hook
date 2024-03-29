//
// The Hook Programming Language
// struct.h
//

#ifndef HK_STRUCT_H
#define HK_STRUCT_H

#include "string.h"

#define HK_STRUCT_MIN_CAPACITY    (1 << 3)
#define HK_STRUCT_MAX_LOAD_FACTOR 0.75

#define hk_instance_get_field(inst, i) ((inst)->values[(i)])

typedef struct
{
  HkString *name;
  int      index;
} HkField;

typedef struct
{
  HK_OBJECT_HEADER
  int      capacity;
  int      mask;
  int      length;
  HkString *name;
  HkField  *fields;
  HkField  **table;
} HkStruct;

typedef struct
{
  HK_OBJECT_HEADER
  HkStruct *ztruct;
  HkValue  values[1];
} HkInstance;

HkStruct *hk_struct_new(HkString *name);
void hk_struct_free(HkStruct *ztruct);
void hk_struct_release(HkStruct *ztruct);
int hk_struct_index_of(HkStruct *ztruct, HkString *name);
bool hk_struct_define_field(HkStruct *ztruct, HkString *name);
bool hk_struct_equal(HkStruct *ztruct1, HkStruct *ztruct2);
HkInstance *hk_instance_new(HkStruct *ztruct);
void hk_instance_free(HkInstance *inst);
void hk_instance_release(HkInstance *inst);
HkInstance *hk_instance_set_field(HkInstance *inst, int index, HkValue value);
void hk_instance_inplace_set_field(HkInstance *inst, int index, HkValue value);
void hk_instance_print(HkInstance *inst);
bool hk_instance_equal(HkInstance *inst1, HkInstance *inst2);

#endif // HK_STRUCT_H
