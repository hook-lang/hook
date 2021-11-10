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
  OBJECT_HEADER
  int capacity;
  int length;
  value_t *elements;
} array_t;

array_t *array_allocate(int min_capacity);
array_t *array_new(int min_capacity);
void array_free(array_t *arr);
int array_index_of(array_t *arr, value_t elem);
array_t *array_add_element(array_t *arr, value_t elem);
array_t *array_set_element(array_t *arr, int index, value_t elem);
array_t *array_delete_element(array_t *arr, int index);
array_t *array_concat(array_t *arr1, array_t *arr2);
array_t *array_diff(array_t *arr1, array_t *arr2);
void array_inplace_add_element(array_t *arr, value_t elem);
void array_inplace_set_element(array_t *arr, int index, value_t elem);
void array_inplace_delete_element(array_t *arr, int index);
void array_inplace_concat(array_t *dest, array_t *src);
void array_inplace_diff(array_t *dest, array_t *src);
void array_print(array_t *arr);
bool array_equal(array_t *arr1, array_t *arr2);
bool array_slice(array_t *arr, int start, int stop, array_t **result);
void array_serialize(array_t *arr, FILE *stream);
array_t *array_deserialize(FILE *stream);

#endif
