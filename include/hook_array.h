//
// Hook Programming Language
// hook_array.h
//

#ifndef HOOK_ARRAY_H
#define HOOK_ARRAY_H

#include "hook_value.h"
#include "hook_iterator.h"

#define HK_ARRAY_MIN_CAPACITY (1 << 3)

typedef struct
{
  HK_OBJECT_HEADER
  int capacity;
  int length;
  hk_value_t *elements;
} hk_array_t;

hk_array_t *hk_array_allocate(int min_capacity);
hk_array_t *hk_array_new(int min_capacity);
void hk_array_free(hk_array_t *arr);
void hk_array_release(hk_array_t *arr);
int hk_array_index_of(hk_array_t *arr, hk_value_t elem);
hk_array_t *hk_array_add_element(hk_array_t *arr, hk_value_t elem);
hk_array_t *hk_array_set_element(hk_array_t *arr, int index, hk_value_t elem);
hk_array_t *hk_array_delete_element(hk_array_t *arr, int index);
hk_array_t *hk_array_concat(hk_array_t *arr1, hk_array_t *arr2);
hk_array_t *hk_array_diff(hk_array_t *arr1, hk_array_t *arr2);
void hk_array_inplace_add_element(hk_array_t *arr, hk_value_t elem);
void hk_array_inplace_set_element(hk_array_t *arr, int index, hk_value_t elem);
void hk_array_inplace_delete_element(hk_array_t *arr, int index);
void hk_array_inplace_concat(hk_array_t *dest, hk_array_t *src);
void hk_array_inplace_diff(hk_array_t *dest, hk_array_t *src);
void hk_array_print(hk_array_t *arr);
bool hk_array_equal(hk_array_t *arr1, hk_array_t *arr2);
int hk_array_compare(hk_array_t *arr1, hk_array_t *arr2, int *result);
bool hk_array_slice(hk_array_t *arr, int start, int stop, hk_array_t **result);
void hk_array_serialize(hk_array_t *arr, FILE *stream);
hk_array_t *hk_array_deserialize(FILE *stream);
hk_iterator_t *hk_array_new_iterator(hk_array_t *arr);

#endif // HOOK_ARRAY_H
