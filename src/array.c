//
// Hook Programming Language
// array.c
//

#include "array.h"
#include <stdlib.h>
#include <stdio.h>
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

array_t *array_allocate(int min_capacity)
{
  array_t *arr = (array_t *) allocate(sizeof(*arr));
  int capacity = ARRAY_MIN_CAPACITY;
  while (capacity < min_capacity)
    capacity <<= 1;
  arr->ref_count = 0;
  arr->capacity = capacity;
  arr->elements = (value_t *) allocate(sizeof(*arr->elements) * capacity);
  return arr;
}

void array_free(array_t *arr)
{
  for (int i = 0; i < arr->length; ++i)
    value_release(arr->elements[i]);
  free(arr->elements);
  free(arr);
}

void array_inplace_add_element(array_t *arr, value_t elem)
{
  resize(arr);
  VALUE_INCR_REF(elem);
  arr->elements[arr->length] = elem;
  ++arr->length;
}

void array_print(array_t *arr)
{
  printf("[");
  if (!arr->length)
  {
    printf("]");
    return;
  }
  value_print(arr->elements[0], true);
  for (int i = 1; i < arr->length; ++i)
  {
    printf(", ");
    value_print(arr->elements[i], true);
  }
  printf("]");
}

bool array_equal(array_t *arr1, array_t *arr2)
{
  if (arr1->length != arr2->length)
    return false;
  for (int i = 0; i < arr1->length; ++i)
    if (!value_equal(arr1->elements[i], arr2->elements[i]))
      return false;  
  return true;
}
