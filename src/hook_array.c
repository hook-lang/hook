//
// Hook Programming Language
// hook_array.c
//

#include "hook_array.h"
#include <stdlib.h>
#include "hook_utils.h"
#include "hook_memory.h"
#include "hook_status.h"

typedef struct
{
  HK_ITERATOR_HEADER
  hk_array_t *iterable;
  int32_t current;
} array_iterator_t;

static inline hk_array_t *array_allocate(int32_t min_capacity);
static void array_iterator_deinit(hk_iterator_t *it);
static bool array_iterator_is_valid(hk_iterator_t *it);
static hk_value_t array_iterator_get_current(hk_iterator_t *it);
static void array_iterator_next(hk_iterator_t *it);

static inline hk_array_t *array_allocate(int32_t min_capacity)
{
  hk_array_t *arr = (hk_array_t *) hk_allocate(sizeof(*arr));
  int32_t capacity = min_capacity < HK_ARRAY_MIN_CAPACITY ? HK_ARRAY_MIN_CAPACITY : min_capacity;
  capacity = hk_power_of_two_ceil(capacity);
  arr->ref_count = 0;
  arr->capacity = capacity;
  arr->elements = (hk_value_t *) hk_allocate(sizeof(*arr->elements) * capacity);
  return arr;
}

static void array_iterator_deinit(hk_iterator_t *it)
{
  hk_array_release(((array_iterator_t *) it)->iterable);
}

static bool array_iterator_is_valid(hk_iterator_t *it)
{
  array_iterator_t *arr_it = (array_iterator_t *) it;
  hk_array_t *arr = arr_it->iterable;
  return arr_it->current < arr->length;
}

static hk_value_t array_iterator_get_current(hk_iterator_t *it)
{
  array_iterator_t *arr_it = (array_iterator_t *) it;
  return arr_it->iterable->elements[arr_it->current];
}

static void array_iterator_next(hk_iterator_t *it)
{
  array_iterator_t *arr_it = (array_iterator_t *) it;
  ++arr_it->current;
}

hk_array_t *hk_array_new(void)
{
  return hk_array_new_with_capacity(0);
}

hk_array_t *hk_array_new_with_capacity(int32_t min_capacity)
{
  hk_array_t *arr = array_allocate(min_capacity);
  arr->length = 0;
  return arr;
}

void hk_array_ensure_capacity(hk_array_t *arr, int32_t min_capacity)
{
  if (min_capacity <= arr->capacity)
    return;
  int32_t capacity = hk_power_of_two_ceil(min_capacity);
  arr->capacity = capacity;
  arr->elements = (hk_value_t *) hk_reallocate(arr->elements,
    sizeof(*arr->elements) * capacity);
}

void hk_array_free(hk_array_t *arr)
{
  for (int32_t i = 0; i < arr->length; ++i)
    hk_value_release(arr->elements[i]);
  free(arr->elements);
  free(arr);
}

void hk_array_release(hk_array_t *arr)
{
  hk_decr_ref(arr);
  if (hk_is_unreachable(arr))
    hk_array_free(arr);
}

int32_t hk_array_index_of(hk_array_t *arr, hk_value_t elem)
{
  for (int32_t i = 0; i < arr->length; ++i)
    if (hk_value_equal(arr->elements[i], elem))
      return i;
  return -1;
}

hk_array_t *hk_array_add_element(hk_array_t *arr, hk_value_t elem)
{
  int32_t length = arr->length;
  hk_array_t *result = array_allocate(length + 1);
  result->length = length + 1;
  for (int32_t i = 0; i < length; i++)
  {
    hk_value_t val = arr->elements[i];
    hk_value_incr_ref(val);
    result->elements[i] = val;
  }
  hk_value_incr_ref(elem);
  result->elements[length] = elem;
  return result;
}

hk_array_t *hk_array_set_element(hk_array_t *arr, int32_t index, hk_value_t elem)
{
  int32_t length = arr->length;
  hk_array_t *result = array_allocate(length);
  result->length = length;
  for (int32_t i = 0; i < index; ++i)
  {
    hk_value_t val = arr->elements[i];
    hk_value_incr_ref(val);
    result->elements[i] = val;
  }
  hk_value_incr_ref(elem);
  result->elements[index] = elem;
  for (int32_t i = index + 1; i < length; ++i)
  {
    hk_value_t val = arr->elements[i];
    hk_value_incr_ref(val);
    result->elements[i] = val;
  }
  return result;
}

hk_array_t *hk_array_insert_element(hk_array_t *arr, int32_t index, hk_value_t elem)
{
  int32_t length = arr->length;
  hk_array_t *result = array_allocate(length + 1);
  result->length = length + 1;
  for (int32_t i = 0; i < index; ++i)
  {
    hk_value_t val = arr->elements[i];
    hk_value_incr_ref(val);
    result->elements[i] = val;
  }
  hk_value_incr_ref(elem);
  result->elements[index] = elem;
  for (int32_t i = index; i < length; ++i)
  {
    hk_value_t val = arr->elements[i];
    hk_value_incr_ref(val);
    result->elements[i + 1] = val;
  }
  return result;
}

hk_array_t *hk_array_delete_element(hk_array_t *arr, int32_t index)
{
  int32_t length = arr->length;
  hk_array_t *result = array_allocate(length - 1);
  result->length = length - 1;
  for (int32_t i = 0; i < index; i++)
  {
    hk_value_t elem = arr->elements[i];
    hk_value_incr_ref(elem);
    result->elements[i] = elem;
  }
  for (int32_t i = index + 1; i < length; i++)
  {
    hk_value_t elem = arr->elements[i];
    hk_value_incr_ref(elem);
    result->elements[i - 1] = elem;
  }
  return result;
}

hk_array_t *hk_array_concat(hk_array_t *arr1, hk_array_t *arr2)
{
  int32_t length = arr1->length + arr2->length;
  hk_array_t *result = array_allocate(length);
  result->length = length;
  int32_t j = 0;
  for (int32_t i = 0; i < arr1->length; ++i, ++j)
  {
    hk_value_t elem = arr1->elements[i];
    hk_value_incr_ref(elem);
    result->elements[j] = elem;
  }
  for (int32_t i = 0; i < arr2->length; ++i, ++j)
  {
    hk_value_t elem = arr2->elements[i];
    hk_value_incr_ref(elem);
    result->elements[j] = elem;
  }
  return result;
}

hk_array_t *hk_array_diff(hk_array_t *arr1, hk_array_t *arr2)
{
  hk_array_t *result = array_allocate(0);
  result->length = 0;
  for (int32_t i = 0; i < arr1->length; ++i)
  {
    hk_value_t elem = arr1->elements[i];
    if (hk_array_index_of(arr2, elem) == -1)
      hk_array_inplace_add_element(result, elem);
  }
  return result;
}

void hk_array_inplace_add_element(hk_array_t *arr, hk_value_t elem)
{
  hk_array_ensure_capacity(arr, arr->length + 1);
  hk_value_incr_ref(elem);
  arr->elements[arr->length] = elem;
  ++arr->length;
}

void hk_array_inplace_set_element(hk_array_t *arr, int32_t index, hk_value_t elem)
{
  hk_value_incr_ref(elem);
  hk_value_release(arr->elements[index]);
  arr->elements[index] = elem;
}

void hk_array_inplace_insert_element(hk_array_t *arr, int32_t index, hk_value_t elem)
{
  hk_array_ensure_capacity(arr, arr->length + 1);
  hk_value_incr_ref(elem);
  for (int32_t i = arr->length; i > index; --i)
    arr->elements[i] = arr->elements[i - 1];
  arr->elements[index] = elem;
  ++arr->length;
}

void hk_array_inplace_delete_element(hk_array_t *arr, int32_t index)
{
  hk_value_release(arr->elements[index]);
  for (int32_t i = index; i < arr->length - 1; ++i)
    arr->elements[i] = arr->elements[i + 1];
  --arr->length;
}

void hk_array_inplace_concat(hk_array_t *dest, hk_array_t *src)
{
  int32_t length = dest->length + src->length;
  hk_array_ensure_capacity(dest, length);
  for (int32_t i = 0; i < src->length; ++i)
  {
    hk_value_t elem = src->elements[i];
    hk_value_incr_ref(elem);
    dest->elements[dest->length] = elem;
    ++dest->length;
  }
}

void hk_array_inplace_diff(hk_array_t *dest, hk_array_t *src)
{
  for (int32_t i = 0; i < src->length; ++i)
  {
    hk_value_t elem = src->elements[i];
    int32_t n = dest->length;
    for (int32_t j = 0; j < n; ++j)
    {
      if (!hk_value_equal(elem, dest->elements[j]))
        continue;
      hk_array_inplace_delete_element(dest, j);
      --n;
    }
  }
}

void hk_array_print(hk_array_t *arr)
{
  printf("[");
  int32_t length = arr->length;
  if (!length)
  {
    printf("]");
    return;
  }
  hk_value_t *elements = arr->elements;
  hk_value_print(elements[0], true);
  for (int32_t i = 1; i < length; ++i)
  {
    printf(", ");
    hk_value_print(elements[i], true);
  }
  printf("]");
}

bool hk_array_equal(hk_array_t *arr1, hk_array_t *arr2)
{
  if (arr1 == arr2)
    return true;
  if (arr1->length != arr2->length)
    return false;
  for (int32_t i = 0; i < arr1->length; ++i)
    if (!hk_value_equal(arr1->elements[i], arr2->elements[i]))
      return false;  
  return true;
}

int32_t hk_array_compare(hk_array_t *arr1, hk_array_t *arr2, int32_t *result)
{
  if (arr1 == arr2)
  {
    *result = 0;
    return HK_STATUS_OK;
  }
  for (int32_t i = 0; i < arr1->length && i < arr2->length; ++i)
  {
    int32_t _result;
    if (hk_value_compare(arr1->elements[i], arr2->elements[i], &_result) == HK_STATUS_ERROR)
      return HK_STATUS_ERROR;
    if (_result)
    {
      *result = _result;
      return HK_STATUS_OK;
    }
  }
  if (arr1->length > arr2->length)
  {
    *result = 1;
    return HK_STATUS_OK;
  }
  if (arr1->length < arr2->length)
  {
    *result = -1;
    return HK_STATUS_OK;
  }
  *result = 0;
  return HK_STATUS_OK;
}

hk_iterator_t *hk_array_new_iterator(hk_array_t *arr)
{
  array_iterator_t *it = (array_iterator_t *) hk_allocate(sizeof(*it));
  hk_iterator_init((hk_iterator_t *) it, &array_iterator_deinit,
    &array_iterator_is_valid, &array_iterator_get_current,
    &array_iterator_next);
  hk_incr_ref(arr);
  it->iterable = arr;
  it->current = 0;
  return (hk_iterator_t *) it;
}

void hk_array_serialize(hk_array_t *arr, FILE *stream)
{
  fwrite(&arr->capacity, sizeof(arr->capacity), 1, stream);
  fwrite(&arr->length, sizeof(arr->length), 1, stream);
  hk_value_t *elements = arr->elements;
  for (int32_t i = 0; i < arr->length; ++i)
    hk_value_serialize(elements[i], stream);
}

hk_array_t *hk_array_deserialize(FILE *stream)
{
  int32_t capacity;
  int32_t length;
  if (fread(&capacity, sizeof(capacity), 1, stream) != 1)
    return NULL;
  if (fread(&length, sizeof(length), 1, stream) != 1)
    return NULL;
  hk_array_t *arr = array_allocate(capacity);
  arr->length = length;
  for (int32_t i = 0; i < length; ++i)
  {
    hk_value_t elem;
    if (!hk_value_deserialize(stream, &elem))
      return NULL;
    hk_value_incr_ref(elem);
    arr->elements[i] = elem;
  }
  return arr;
}
