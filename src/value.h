//
// Hook Programming Language
// value.h
//

#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>

#define FLAG_NONE   0b00
#define FLAG_OBJECT 0b01
#define FLAG_FALSEY 0b10

#define NULL_VALUE      ((value_t) {.type = TYPE_NULL, .flags = FLAG_FALSEY})
#define FALSE_VALUE     ((value_t) {.type = TYPE_BOOLEAN, .flags = FLAG_FALSEY, .as_boolean = false})
#define TRUE_VALUE      ((value_t) {.type = TYPE_BOOLEAN, .flags = FLAG_NONE, .as_boolean = true})
#define NUMBER_VALUE(n) ((value_t) {.type = TYPE_NUMBER, .flags = FLAG_NONE, .as_number = (n)})
#define STRING_VALUE(s) ((value_t) {.type = TYPE_STRING, .flags = FLAG_OBJECT, .as_pointer = (s)})
#define ARRAY_VALUE(a)  ((value_t) {.type = TYPE_ARRAY, .flags = FLAG_OBJECT, .as_pointer = (a)})

#define IS_NULL(v)    ((v).type == TYPE_NULL)
#define IS_BOOLEAN(v) ((v).type == TYPE_BOOLEAN)
#define IS_NUMBER(v)  ((v).type == TYPE_NUMBER)
#define IS_STRING(v)  ((v).type == TYPE_STRING)
#define IS_ARRAY(v)   ((v).type == TYPE_ARRAY)
#define IS_OBJECT(v)  ((v).flags & FLAG_OBJECT)
#define IS_FALSEY(v)  ((v).flags & FLAG_FALSEY)
#define IS_TRUTHY(v)  (!IS_FALSEY(v))

#define AS_STRING(v) ((string_t *) (v).as_pointer)
#define AS_ARRAY(v)  ((array_t *) (v).as_pointer)
#define AS_OBJECT(v) ((object_t *) (v).as_pointer)

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
  TYPE_ARRAY
} type_t;

typedef struct
{
  type_t type;
  int flags;
  union
  {
    bool as_boolean;
    double as_number;
    void *as_pointer;
  };
} value_t;

typedef struct
{
  OBJECT_HEADER
} object_t;

const char *type_name(type_t type);
void value_free(value_t val);
void value_release(value_t val);
void value_print(value_t val, bool quoted);
bool value_equal(value_t val1, value_t val2);
int value_compare(value_t val1, value_t val2);

#endif
