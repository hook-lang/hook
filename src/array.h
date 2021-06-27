//
// Hook Programming Language
// array.h
//

#ifndef ARRAY_H
#define ARRAY_H

#include "value.h"

#define ARRAY_MIN_CAPACITY 8

typedef struct
{
  int capacity;
  int length;
  value_t *elements;
} array_t;

array_t *array_new(int min_capacity);
void array_free(array_t *arr);
void array_add_element(array_t *arr, value_t val);

#endif
