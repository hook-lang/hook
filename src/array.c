//
// Hook Programming Language
// array.c
//

#include "array.h"
#include <stdlib.h>
#include "memory.h"

static inline void resize(array_t *arr);

static inline void resize(array_t *arr)
{
  if (arr->length < arr->capacity)
    return;
  int capacity = arr->capacity << 1;
  arr->capacity = capacity;
  arr->elements = (value_t *) reallocate(arr->elements,
    sizeof(*arr->elements) * capacity);
}

array_t *array_new(int min_capacity)
{
  array_t *arr = (array_t *) allocate(sizeof(*arr));
  int capacity = ARRAY_MIN_CAPACITY;
  while (capacity < min_capacity)
    capacity <<= 1;
  arr->capacity = capacity;
  arr->length = 0;
  arr->elements = (value_t *) allocate(sizeof(arr->elements) * capacity);
  return arr;
}

void array_free(array_t *arr)
{
  for (int i = 0; i < arr->length; ++i)
    value_release(arr->elements[i]);
  free(arr->elements);
  free(arr);
}

void array_add_element(array_t *arr, value_t val)
{
  resize(arr);
  VALUE_INCR_REF(val);
  arr->elements[arr->length] = val;
  ++arr->length;
}
