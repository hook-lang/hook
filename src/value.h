//
// Hook Programming Language
// value.h
//

#ifndef VALUE_H
#define VALUE_H

#define NULL_VALUE      ((value_t) {.type = TYPE_NULL})
#define NUMBER_VALUE(n) ((value_t) {.type = TYPE_NUMBER, .as_number = (n)})

#define IS_NULL(v)   ((v).type == TYPE_NULL)
#define IS_NUMBER(v) ((v).type == TYPE_NUMBER)

typedef enum
{
  TYPE_NULL,
  TYPE_NUMBER
} type_t;

typedef struct
{
  type_t type;
  double as_number;
} value_t; 

const char *type_name(type_t type);

#endif
