//
// Hook Programming Language
// hook_value.h
//

#ifndef HOOK_VALUE_H
#define HOOK_VALUE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define HK_TYPE_NIL      0x00
#define HK_TYPE_BOOL     0x01
#define HK_TYPE_FLOAT    0x02
#define HK_TYPE_STRING   0x03
#define HK_TYPE_RANGE    0x04
#define HK_TYPE_ARRAY    0x05
#define HK_TYPE_STRUCT   0x06
#define HK_TYPE_INSTANCE 0x07
#define HK_TYPE_ITERATOR 0x08
#define HK_TYPE_CALLABLE 0x09
#define HK_TYPE_USERDATA 0x0a

#define HK_FLAG_NONE     0x00
#define HK_FLAG_OBJECT   0x01
#define HK_FLAG_FALSEY   0x02
#define HK_FLAG_HASHABLE 0x04
#define HK_FLAG_ITERABLE 0x08
#define HK_FLAG_NATIVE   0x10

#define HK_NIL_VALUE         ((hk_value_t) {.type = HK_TYPE_NIL, .flags = HK_FLAG_FALSEY})
#define HK_FALSE_VALUE       ((hk_value_t) {.type = HK_TYPE_BOOL, .flags = HK_FLAG_FALSEY, .as_bool = false})
#define HK_TRUE_VALUE        ((hk_value_t) {.type = HK_TYPE_BOOL, .flags = HK_FLAG_NONE, .as_bool = true})
#define hk_float_value(n)    ((hk_value_t) {.type = HK_TYPE_FLOAT, .flags = HK_FLAG_NONE, .as_float = (n)})
#define hk_string_value(s)   ((hk_value_t) {.type = HK_TYPE_STRING, .flags = HK_FLAG_OBJECT | HK_FLAG_HASHABLE, .as_pointer = (s)})
#define hk_range_value(r)    ((hk_value_t) {.type = HK_TYPE_RANGE, .flags = HK_FLAG_OBJECT | HK_FLAG_ITERABLE, .as_pointer = (r)})
#define hk_array_value(a)    ((hk_value_t) {.type = HK_TYPE_ARRAY, .flags = HK_FLAG_OBJECT | HK_FLAG_ITERABLE, .as_pointer = (a)})
#define hk_struct_value(s)   ((hk_value_t) {.type = HK_TYPE_STRUCT, .flags = HK_FLAG_OBJECT, .as_pointer = (s)})
#define hk_instance_value(i) ((hk_value_t) {.type = HK_TYPE_INSTANCE, .flags = HK_FLAG_OBJECT, .as_pointer = (i)})
#define hk_iterator_value(i) ((hk_value_t) {.type = HK_TYPE_ITERATOR, .flags = HK_FLAG_OBJECT, .as_pointer = (i)})
#define hk_closure_value(c)  ((hk_value_t) {.type = HK_TYPE_CALLABLE, .flags = HK_FLAG_OBJECT, .as_pointer = (c)})
#define hk_native_value(n)   ((hk_value_t) {.type = HK_TYPE_CALLABLE, .flags = HK_FLAG_OBJECT | HK_FLAG_NATIVE, .as_pointer = (n)})
#define hk_userdata_value(u) ((hk_value_t) {.type = HK_TYPE_USERDATA, .flags = HK_FLAG_OBJECT, .as_pointer = (u)})

#define hk_is_nil(v)      ((v).type == HK_TYPE_NIL)
#define hk_is_bool(v)     ((v).type == HK_TYPE_BOOL)
#define hk_is_float(v)    ((v).type == HK_TYPE_FLOAT)
#define hk_is_int(v)      (hk_is_float(v) && (v).as_float == (int64_t) (v).as_float)
#define hk_is_string(v)   ((v).type == HK_TYPE_STRING)
#define hk_is_range(v)    ((v).type == HK_TYPE_RANGE)
#define hk_is_array(v)    ((v).type == HK_TYPE_ARRAY)
#define hk_is_struct(v)   ((v).type == HK_TYPE_STRUCT)
#define hk_is_instance(v) ((v).type == HK_TYPE_INSTANCE)
#define hk_is_iterator(v) ((v).type == HK_TYPE_ITERATOR)
#define hk_is_callable(v) ((v).type == HK_TYPE_CALLABLE)
#define hk_is_userdata(v) ((v).type == HK_TYPE_USERDATA)
#define hk_is_object(v)   ((v).flags & HK_FLAG_OBJECT)
#define hk_is_falsey(v)   ((v).flags & HK_FLAG_FALSEY)
#define hk_is_truthy(v)   (!hk_is_falsey(v))
#define hk_is_hashable(v) ((v).flags & HK_FLAG_HASHABLE)
#define hk_is_iterable(v) ((v).flags & HK_FLAG_ITERABLE)
#define hk_is_native(v)   ((v).flags & HK_FLAG_NATIVE)

#define hk_as_string(v)   ((hk_string_t *) (v).as_pointer)
#define hk_as_range(v)    ((hk_range_t *) (v).as_pointer)
#define hk_as_array(v)    ((hk_array_t *) (v).as_pointer)
#define hk_as_struct(v)   ((hk_struct_t *) (v).as_pointer)
#define hk_as_instance(v) ((hk_instance_t *) (v).as_pointer)
#define hk_as_iterator(v) ((hk_iterator_t *) (v).as_pointer)
#define hk_as_closure(v)  ((hk_closure_t *) (v).as_pointer)
#define hk_as_native(v)   ((hk_native_t *) (v).as_pointer)
#define hk_as_userdata(v) ((hk_userdata_t *) (v).as_pointer)
#define hk_as_object(v)   ((hk_object_t *) (v).as_pointer)

#define HK_OBJECT_HEADER int32_t ref_count;

#define hk_incr_ref(o)       ++(o)->ref_count
#define hk_decr_ref(o)       --(o)->ref_count
#define hk_is_unreachable(o) (!(o)->ref_count)

#define hk_value_incr_ref(v) if (hk_is_object(v)) hk_incr_ref(hk_as_object(v))
#define hk_value_decr_ref(v) if (hk_is_object(v)) hk_decr_ref(hk_as_object(v))

typedef struct
{
  int32_t type;
  int32_t flags;
  union
  {
    bool as_bool;
    double as_float;
    void *as_pointer;
  };
} hk_value_t;

typedef struct
{
  HK_OBJECT_HEADER
} hk_object_t;

const char *hk_type_name(int32_t type);
void hk_value_release(hk_value_t val);
void hk_value_print(hk_value_t val, bool quoted);
bool hk_value_equal(hk_value_t val1, hk_value_t val2);
int32_t hk_value_compare(hk_value_t val1, hk_value_t val2, int32_t *result);
void hk_value_serialize(hk_value_t val, FILE *stream);
bool hk_value_deserialize(FILE *stream, hk_value_t *result);

#endif // HOOK_VALUE_H
