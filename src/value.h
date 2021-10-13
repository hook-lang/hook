//
// Hook Programming Language
// value.h
//

#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>

#define FLAG_NONE   0b0000
#define FLAG_OBJECT 0b0001
#define FLAG_FALSEY 0b0010
#define FLAG_NATIVE 0b0100

#define NULL_VALUE        ((value_t) {.type = TYPE_NULL, .flags = FLAG_FALSEY})
#define FALSE_VALUE       ((value_t) {.type = TYPE_BOOLEAN, .flags = FLAG_FALSEY, .as.boolean = false})
#define TRUE_VALUE        ((value_t) {.type = TYPE_BOOLEAN, .flags = FLAG_NONE, .as.boolean = true})
#define NUMBER_VALUE(n)   ((value_t) {.type = TYPE_NUMBER, .flags = FLAG_NONE, .as.number = (n)})
#define STRING_VALUE(s)   ((value_t) {.type = TYPE_STRING, .flags = FLAG_OBJECT, .as.pointer = (s)})
#define ARRAY_VALUE(a)    ((value_t) {.type = TYPE_ARRAY, .flags = FLAG_OBJECT, .as.pointer = (a)})
#define FUNCTION_VALUE(f) ((value_t) {.type = TYPE_CALLABLE, .flags = FLAG_OBJECT, .as.pointer = (f)})
#define NATIVE_VALUE(n)   ((value_t) {.type = TYPE_CALLABLE, .flags = FLAG_OBJECT | FLAG_NATIVE, .as.pointer = (n)})

#define IS_NULL(v)     ((v).type == TYPE_NULL)
#define IS_BOOLEAN(v)  ((v).type == TYPE_BOOLEAN)
#define IS_NUMBER(v)   ((v).type == TYPE_NUMBER)
#define IS_INTEGER(v)  (IS_NUMBER(v) && (v).as.number == (long) (v).as.number)
#define IS_STRING(v)   ((v).type == TYPE_STRING)
#define IS_ARRAY(v)    ((v).type == TYPE_ARRAY)
#define IS_CALLABLE(v) ((v).type == TYPE_CALLABLE)
#define IS_OBJECT(v)  ((v).flags & FLAG_OBJECT)
#define IS_FALSEY(v)  ((v).flags & FLAG_FALSEY)
#define IS_TRUTHY(v)  (!IS_FALSEY(v))
#define IS_NATIVE(v)  ((v).flags & FLAG_NATIVE)

#define AS_STRING(v)   ((string_t *) (v).as.pointer)
#define AS_ARRAY(v)    ((array_t *) (v).as.pointer)
#define AS_CALLABLE(v) ((callable_t *) (v).as.pointer)
#define AS_FUNCTION(v) ((function_t *) (v).as.pointer)
#define AS_NATIVE(v)   ((native_t *) (v).as.pointer)
#define AS_OBJECT(v)   ((object_t *) (v).as.pointer)

#define OBJECT_HEADER int ref_count;

#define INCR_REF(o)       ++(o)->ref_count
#define DECR_REF(o)       --(o)->ref_count
#define IS_UNREACHABLE(o) (!(o)->ref_count)

#define VALUE_INCR_REF(v) if (IS_OBJECT(v)) INCR_REF(AS_OBJECT(v))
#define VALUE_DECR_REF(v) if (IS_OBJECT(v)) DECR_REF(AS_OBJECT(v))

typedef enum
{
  TYPE_NULL,
  TYPE_BOOLEAN,
  TYPE_NUMBER,
  TYPE_STRING,
  TYPE_ARRAY,
  TYPE_CALLABLE
} type_t;

typedef struct
{
  type_t type;
  int flags;
  union
  {
    bool boolean;
    double number;
    void *pointer;
  } as;
} value_t;

typedef struct
{
  OBJECT_HEADER
} object_t;

const char *type_name(type_t type);
void value_release(value_t val);
void value_print(value_t val, bool quoted);
bool value_equal(value_t val1, value_t val2);
int value_compare(value_t val1, value_t val2);

#endif
