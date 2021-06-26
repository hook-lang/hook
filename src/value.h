//
// Hook Programming Language
// value.h
//

#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>

#define NULL_VALUE       ((value_t) {.type = TYPE_NULL})
#define BOOLEAN_VALUE(b) ((value_t) {.type = TYPE_BOOLEAN, .as_boolean = (b)})
#define NUMBER_VALUE(n)  ((value_t) {.type = TYPE_NUMBER, .as_number = (n)})

#define IS_NULL(v)    ((v).type == TYPE_NULL)
#define IS_BOOLEAN(v) ((v).type == TYPE_BOOLEAN)
#define IS_NUMBER(v)  ((v).type == TYPE_NUMBER)

typedef enum
{
  TYPE_NULL,
  TYPE_BOOLEAN,
  TYPE_NUMBER
} type_t;

typedef struct
{
  type_t type;
  union
  {
    bool as_boolean;
    double as_number;
  };
} value_t; 

const char *type_name(type_t type);

#endif
