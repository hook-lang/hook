//
// array.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_ARRAY_H
#define HK_ARRAY_H

#include "iterator.h"
#include "value.h"

#define HK_ARRAY_MIN_CAPACITY (1 << 3)

#define hk_array_is_empty(a)       (!(a)->length)
#define hk_array_get_element(a, i) ((a)->elements[(i)])

typedef struct
{
  HK_OBJECT_HEADER
  int     capacity;
  int     length;
  HkValue *elements;
} HkArray;

HkArray *hk_array_new(void);
HkArray *hk_array_new_with_capacity(int minCapacity);
void hk_array_ensure_capacity(HkArray *arr, int minCapacity);
void hk_array_free(HkArray *arr);
void hk_array_release(HkArray *arr);
int hk_array_index_of(HkArray *arr, HkValue elem);
HkArray *hk_array_add_element(HkArray *arr, HkValue elem);
HkArray *hk_array_set_element(HkArray *arr, int index, HkValue elem);
HkArray *hk_array_insert_element(HkArray *arr, int index, HkValue elem);
HkArray *hk_array_delete_element(HkArray *arr, int index);
HkArray *hk_array_concat(HkArray *arr1, HkArray *arr2);
HkArray *hk_array_diff(HkArray *arr1, HkArray *arr2);
void hk_array_inplace_add_element(HkArray *arr, HkValue elem);
void hk_array_inplace_set_element(HkArray *arr, int index, HkValue elem);
void hk_array_inplace_insert_element(HkArray *arr, int index, HkValue elem);
void hk_array_inplace_delete_element(HkArray *arr, int index);
void hk_array_inplace_concat(HkArray *dest, HkArray *src);
void hk_array_inplace_diff(HkArray *dest, HkArray *src);
void hk_array_inplace_clear(HkArray *arr);
void hk_array_print(HkArray *arr);
bool hk_array_equal(HkArray *arr1, HkArray *arr2);
bool hk_array_compare(HkArray *arr1, HkArray *arr2, int *result);
HkIterator *hk_array_new_iterator(HkArray *arr);
HkArray *hk_array_reverse(HkArray *arr);
bool hk_array_sort(HkArray *arr, HkArray **result);
void hk_array_serialize(HkArray *arr, FILE *stream);
HkArray *hk_array_deserialize(FILE *stream);

#endif // HK_ARRAY_H
