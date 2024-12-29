//
// value.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_VALUE_H
#define HK_VALUE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum
{
  HK_TYPE_NIL,
  HK_TYPE_BOOL,
  HK_TYPE_NUMBER,
  HK_TYPE_STRING,
  HK_TYPE_RANGE,
  HK_TYPE_ARRAY,
  HK_TYPE_STRUCT,
  HK_TYPE_INSTANCE,
  HK_TYPE_ITERATOR,
  HK_TYPE_CALLABLE,
  HK_TYPE_USERDATA
} HkType;

#define HK_FLAG_NONE       0x00
#define HK_FLAG_OBJECT     0x01
#define HK_FLAG_FALSEY     0x02
#define HK_FLAG_COMPARABLE 0x04
#define HK_FLAG_ITERABLE   0x08
#define HK_FLAG_NATIVE     0x10

#define HK_NIL_VALUE         ((HkValue) { .type = HK_TYPE_NIL, .flags = HK_FLAG_FALSEY | HK_FLAG_COMPARABLE })
#define HK_FALSE_VALUE       ((HkValue) { .type = HK_TYPE_BOOL, .flags = HK_FLAG_FALSEY | HK_FLAG_COMPARABLE, .as.boolean = false })
#define HK_TRUE_VALUE        ((HkValue) { .type = HK_TYPE_BOOL, .flags = HK_FLAG_COMPARABLE, .as.boolean = true })
#define hk_number_value(n)   ((HkValue) { .type = HK_TYPE_NUMBER, .flags = HK_FLAG_COMPARABLE, .as.number = (n) })
#define hk_string_value(s)   ((HkValue) { .type = HK_TYPE_STRING, .flags = HK_FLAG_OBJECT | HK_FLAG_COMPARABLE, .as.pointer = (s) })
#define hk_range_value(r)    ((HkValue) { .type = HK_TYPE_RANGE, .flags = HK_FLAG_OBJECT | HK_FLAG_COMPARABLE | HK_FLAG_ITERABLE, .as.pointer = (r) })
#define hk_array_value(a)    ((HkValue) { .type = HK_TYPE_ARRAY, .flags = HK_FLAG_OBJECT | HK_FLAG_COMPARABLE | HK_FLAG_ITERABLE, .as.pointer = (a) })
#define hk_struct_value(s)   ((HkValue) { .type = HK_TYPE_STRUCT, .flags = HK_FLAG_OBJECT, .as.pointer = (s) })
#define hk_instance_value(i) ((HkValue) { .type = HK_TYPE_INSTANCE, .flags = HK_FLAG_OBJECT, .as.pointer = (i) })
#define hk_iterator_value(i) ((HkValue) { .type = HK_TYPE_ITERATOR, .flags = HK_FLAG_OBJECT, .as.pointer = (i) })
#define hk_closure_value(c)  ((HkValue) { .type = HK_TYPE_CALLABLE, .flags = HK_FLAG_OBJECT, .as.pointer = (c) })
#define hk_native_value(n)   ((HkValue) { .type = HK_TYPE_CALLABLE, .flags = HK_FLAG_OBJECT | HK_FLAG_NATIVE, .as.pointer = (n) })
#define hk_userdata_value(u) ((HkValue) { .type = HK_TYPE_USERDATA, .flags = HK_FLAG_OBJECT, .as.pointer = (u) })

#define hk_as_bool(v)     ((v).as.boolean)
#define hk_as_number(v)   ((v).as.number)
#define hk_as_string(v)   ((HkString *) (v).as.pointer)
#define hk_as_range(v)    ((HkRange *) (v).as.pointer)
#define hk_as_array(v)    ((HkArray *) (v).as.pointer)
#define hk_as_struct(v)   ((HkStruct *) (v).as.pointer)
#define hk_as_instance(v) ((HkInstance *) (v).as.pointer)
#define hk_as_iterator(v) ((HkIterator *) (v).as.pointer)
#define hk_as_closure(v)  ((HkClosure *) (v).as.pointer)
#define hk_as_native(v)   ((HkNative *) (v).as.pointer)
#define hk_as_userdata(v) ((HkUserdata *) (v).as.pointer)
#define hk_as_object(v)   ((HkObject *) (v).as.pointer)

#define hk_is_nil(v)        ((v).type == HK_TYPE_NIL)
#define hk_is_bool(v)       ((v).type == HK_TYPE_BOOL)
#define hk_is_number(v)     ((v).type == HK_TYPE_NUMBER)
#define hk_is_int(v)        (hk_is_number(v) && hk_as_number(v) == (int64_t) hk_as_number(v))
#define hk_is_string(v)     ((v).type == HK_TYPE_STRING)
#define hk_is_range(v)      ((v).type == HK_TYPE_RANGE)
#define hk_is_array(v)      ((v).type == HK_TYPE_ARRAY)
#define hk_is_struct(v)     ((v).type == HK_TYPE_STRUCT)
#define hk_is_instance(v)   ((v).type == HK_TYPE_INSTANCE)
#define hk_is_iterator(v)   ((v).type == HK_TYPE_ITERATOR)
#define hk_is_callable(v)   ((v).type == HK_TYPE_CALLABLE)
#define hk_is_userdata(v)   ((v).type == HK_TYPE_USERDATA)
#define hk_is_object(v)     ((v).flags & HK_FLAG_OBJECT)
#define hk_is_falsey(v)     ((v).flags & HK_FLAG_FALSEY)
#define hk_is_truthy(v)     (!hk_is_falsey(v))
#define hk_is_comparable(v) ((v).flags & HK_FLAG_COMPARABLE)
#define hk_is_iterable(v)   ((v).flags & HK_FLAG_ITERABLE)
#define hk_is_native(v)     ((v).flags & HK_FLAG_NATIVE)

#define HK_OBJECT_HEADER int refCount;

#define hk_incr_ref(o)       ++(o)->refCount
#define hk_decr_ref(o)       --(o)->refCount
#define hk_is_unreachable(o) (!(o)->refCount)

#define hk_value_incr_ref(v) if (hk_is_object(v)) hk_incr_ref(hk_as_object(v))
#define hk_value_decr_ref(v) if (hk_is_object(v)) hk_decr_ref(hk_as_object(v))

typedef struct
{
  HkType   type;
  int      flags;
  union
  {
    bool   boolean;
    double number;
    void   *pointer;
  } as;
} HkValue;

typedef struct
{
  HK_OBJECT_HEADER
} HkObject;

const char *hk_type_name(HkType type);
void hk_value_free(HkValue val);
void hk_value_release(HkValue val);
void hk_value_print(HkValue val, bool quoted);
bool hk_value_equal(HkValue val1, HkValue val2);
bool hk_value_compare(HkValue val1, HkValue val2, int *result);
void hk_value_serialize(HkValue val, FILE *stream);
bool hk_value_deserialize(FILE *stream, HkValue *result);

#endif // HK_VALUE_H
