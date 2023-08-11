//
// The Hook Programming Language
// array.c
//

#include <hook/array.h>
#include <stdlib.h>
#include <hook/memory.h>
#include <hook/utils.h>

typedef struct
{
  HK_ITERATOR_HEADER
  HkArray *arr;
  int current;
} ArrayIterator;

static inline HkArray *array_allocate(int minCapacity);
static inline ArrayIterator *array_iterator_allocate(HkArray *arr);
static void array_iterator_deinit(HkIterator *it);
static bool array_iterator_is_valid(HkIterator *it);
static HkValue array_iterator_get_current(HkIterator *it);
static HkIterator *array_iterator_next(HkIterator *it);
static void array_iterator_inplace_next(HkIterator *it);

static inline HkArray *array_allocate(int minCapacity)
{
  HkArray *arr = (HkArray *) hk_allocate(sizeof(*arr));
  int capacity = minCapacity < HK_ARRAY_MIN_CAPACITY ? HK_ARRAY_MIN_CAPACITY : minCapacity;
  capacity = hk_power_of_two_ceil(capacity);
  arr->ref_count = 0;
  arr->capacity = capacity;
  arr->elements = (HkValue *) hk_allocate(sizeof(*arr->elements) * capacity);
  return arr;
}

static inline ArrayIterator *array_iterator_allocate(HkArray *arr)
{
  ArrayIterator *arr_it = (ArrayIterator *) hk_allocate(sizeof(*arr_it));
  hk_iterator_init((HkIterator *) arr_it, &array_iterator_deinit,
    &array_iterator_is_valid, &array_iterator_get_current,
    &array_iterator_next, &array_iterator_inplace_next);
  hk_incr_ref(arr);
  arr_it->arr = arr;
  return arr_it;
}

static void array_iterator_deinit(HkIterator *it)
{
  hk_array_release(((ArrayIterator *) it)->arr);
}

static bool array_iterator_is_valid(HkIterator *it)
{
  ArrayIterator *arr_it = (ArrayIterator *) it;
  HkArray *arr = arr_it->arr;
  return arr_it->current < arr->length;
}

static HkValue array_iterator_get_current(HkIterator *it)
{
  ArrayIterator *arr_it = (ArrayIterator *) it;
  return arr_it->arr->elements[arr_it->current];
}

static HkIterator *array_iterator_next(HkIterator *it)
{
  ArrayIterator *arr_it = (ArrayIterator *) it;
  ArrayIterator *result = array_iterator_allocate(arr_it->arr);
  result->current = arr_it->current + 1;
  return (HkIterator *) result;
}

static void array_iterator_inplace_next(HkIterator *it)
{
  ArrayIterator *arr_it = (ArrayIterator *) it;
  ++arr_it->current;
}

HkArray *hk_array_new(void)
{
  return hk_array_new_with_capacity(0);
}

HkArray *hk_array_new_with_capacity(int minCapacity)
{
  HkArray *arr = array_allocate(minCapacity);
  arr->length = 0;
  return arr;
}

void hk_array_ensure_capacity(HkArray *arr, int minCapacity)
{
  if (minCapacity <= arr->capacity)
    return;
  int capacity = hk_power_of_two_ceil(minCapacity);
  arr->capacity = capacity;
  arr->elements = (HkValue *) hk_reallocate(arr->elements,
    sizeof(*arr->elements) * capacity);
}

void hk_array_free(HkArray *arr)
{
  for (int i = 0; i < arr->length; ++i)
    hk_value_release(arr->elements[i]);
  free(arr->elements);
  free(arr);
}

void hk_array_release(HkArray *arr)
{
  hk_decr_ref(arr);
  if (hk_is_unreachable(arr))
    hk_array_free(arr);
}

int hk_array_index_of(HkArray *arr, HkValue elem)
{
  for (int i = 0; i < arr->length; ++i)
    if (hk_value_equal(arr->elements[i], elem))
      return i;
  return -1;
}

HkArray *hk_array_add_element(HkArray *arr, HkValue elem)
{
  int length = arr->length;
  HkArray *result = array_allocate(length + 1);
  result->length = length + 1;
  for (int i = 0; i < length; ++i)
  {
    HkValue val = arr->elements[i];
    hk_value_incr_ref(val);
    result->elements[i] = val;
  }
  hk_value_incr_ref(elem);
  result->elements[length] = elem;
  return result;
}

HkArray *hk_array_set_element(HkArray *arr, int index, HkValue elem)
{
  int length = arr->length;
  HkArray *result = array_allocate(length);
  result->length = length;
  for (int i = 0; i < index; ++i)
  {
    HkValue val = arr->elements[i];
    hk_value_incr_ref(val);
    result->elements[i] = val;
  }
  hk_value_incr_ref(elem);
  result->elements[index] = elem;
  for (int i = index + 1; i < length; ++i)
  {
    HkValue val = arr->elements[i];
    hk_value_incr_ref(val);
    result->elements[i] = val;
  }
  return result;
}

HkArray *hk_array_insert_element(HkArray *arr, int index, HkValue elem)
{
  int length = arr->length;
  HkArray *result = array_allocate(length + 1);
  result->length = length + 1;
  for (int i = 0; i < index; ++i)
  {
    HkValue val = arr->elements[i];
    hk_value_incr_ref(val);
    result->elements[i] = val;
  }
  hk_value_incr_ref(elem);
  result->elements[index] = elem;
  for (int i = index; i < length; ++i)
  {
    HkValue val = arr->elements[i];
    hk_value_incr_ref(val);
    result->elements[i + 1] = val;
  }
  return result;
}

HkArray *hk_array_delete_element(HkArray *arr, int index)
{
  int length = arr->length;
  HkArray *result = array_allocate(length - 1);
  result->length = length - 1;
  for (int i = 0; i < index; ++i)
  {
    HkValue elem = arr->elements[i];
    hk_value_incr_ref(elem);
    result->elements[i] = elem;
  }
  for (int i = index + 1; i < length; ++i)
  {
    HkValue elem = arr->elements[i];
    hk_value_incr_ref(elem);
    result->elements[i - 1] = elem;
  }
  return result;
}

HkArray *hk_array_concat(HkArray *arr1, HkArray *arr2)
{
  int length = arr1->length + arr2->length;
  HkArray *result = array_allocate(length);
  result->length = length;
  int j = 0;
  for (int i = 0; i < arr1->length; ++i, ++j)
  {
    HkValue elem = arr1->elements[i];
    hk_value_incr_ref(elem);
    result->elements[j] = elem;
  }
  for (int i = 0; i < arr2->length; ++i, ++j)
  {
    HkValue elem = arr2->elements[i];
    hk_value_incr_ref(elem);
    result->elements[j] = elem;
  }
  return result;
}

HkArray *hk_array_diff(HkArray *arr1, HkArray *arr2)
{
  HkArray *result = array_allocate(0);
  result->length = 0;
  for (int i = 0; i < arr1->length; ++i)
  {
    HkValue elem = arr1->elements[i];
    if (hk_array_index_of(arr2, elem) == -1)
      hk_array_inplace_add_element(result, elem);
  }
  return result;
}

void hk_array_inplace_add_element(HkArray *arr, HkValue elem)
{
  hk_array_ensure_capacity(arr, arr->length + 1);
  hk_value_incr_ref(elem);
  arr->elements[arr->length] = elem;
  ++arr->length;
}

void hk_array_inplace_set_element(HkArray *arr, int index, HkValue elem)
{
  hk_value_incr_ref(elem);
  hk_value_release(arr->elements[index]);
  arr->elements[index] = elem;
}

void hk_array_inplace_insert_element(HkArray *arr, int index, HkValue elem)
{
  hk_array_ensure_capacity(arr, arr->length + 1);
  hk_value_incr_ref(elem);
  for (int i = arr->length; i > index; --i)
    arr->elements[i] = arr->elements[i - 1];
  arr->elements[index] = elem;
  ++arr->length;
}

void hk_array_inplace_delete_element(HkArray *arr, int index)
{
  hk_value_release(arr->elements[index]);
  for (int i = index; i < arr->length - 1; ++i)
    arr->elements[i] = arr->elements[i + 1];
  --arr->length;
}

void hk_array_inplace_concat(HkArray *dest, HkArray *src)
{
  int length = dest->length + src->length;
  hk_array_ensure_capacity(dest, length);
  for (int i = 0; i < src->length; ++i)
  {
    HkValue elem = src->elements[i];
    hk_value_incr_ref(elem);
    dest->elements[dest->length] = elem;
    ++dest->length;
  }
}

void hk_array_inplace_diff(HkArray *dest, HkArray *src)
{
  for (int i = 0; i < src->length; ++i)
  {
    HkValue elem = src->elements[i];
    int n = dest->length;
    for (int j = 0; j < n; ++j)
    {
      if (!hk_value_equal(elem, dest->elements[j]))
        continue;
      hk_array_inplace_delete_element(dest, j);
      --n;
    }
  }
}

void hk_array_inplace_clear(HkArray *arr)
{
  for (int i = 0; i < arr->length; ++i)
    hk_value_release(arr->elements[i]);
  arr->length = 0;
}

void hk_array_print(HkArray *arr)
{
  printf("[");
  int length = arr->length;
  if (!length)
  {
    printf("]");
    return;
  }
  HkValue *elements = arr->elements;
  hk_value_print(elements[0], true);
  for (int i = 1; i < length; ++i)
  {
    printf(", ");
    hk_value_print(elements[i], true);
  }
  printf("]");
}

bool hk_array_equal(HkArray *arr1, HkArray *arr2)
{
  if (arr1 == arr2)
    return true;
  if (arr1->length != arr2->length)
    return false;
  for (int i = 0; i < arr1->length; ++i)
    if (!hk_value_equal(arr1->elements[i], arr2->elements[i]))
      return false;  
  return true;
}

bool hk_array_compare(HkArray *arr1, HkArray *arr2, int *result)
{
  if (arr1 == arr2)
  {
    *result = 0;
    return true;
  }
  for (int i = 0; i < arr1->length && i < arr2->length; ++i)
  {
    int comp;
    if (!hk_value_compare(arr1->elements[i], arr2->elements[i], &comp))
      return false;
    if (!comp)
      continue;
    *result = comp;
    return true;
  }
  if (arr1->length > arr2->length)
  {
    *result = 1;
    return true;
  }
  if (arr1->length < arr2->length)
  {
    *result = -1;
    return true;
  }
  *result = 0;
  return true;
}

HkIterator *hk_array_new_iterator(HkArray *arr)
{
  ArrayIterator *arr_it = array_iterator_allocate(arr);
  arr_it->current = 0;
  return (HkIterator *) arr_it;
}

HkArray *hk_array_reverse(HkArray *arr)
{
  int length = arr->length;
  HkArray *result = array_allocate(length);
  result->length = length;
  for (int i = 0; i < length; ++i)
  {
    HkValue elem = arr->elements[length - i - 1];
    hk_value_incr_ref(elem);
    result->elements[i] = elem;
  }
  return result;
}

bool hk_array_sort(HkArray *arr, HkArray **result)
{
  int length = arr->length;
  HkArray *_result = array_allocate(length);
  _result->length = 0;
  for (int i = 0; i < length; ++i)
  {
    HkValue elem1 = arr->elements[i];
    int index = 0;
    for (; index < _result->length; ++index)
    {
      HkValue elem2 = _result->elements[index];
      int comp;
      if (!hk_value_compare(elem1, elem2, &comp))
      {
        hk_array_free(_result);
        return false;
      }
      if (comp < 0)
        break;
    }
    hk_array_inplace_insert_element(_result, index, elem1);
  }
  *result = _result;
  return true;
}

void hk_array_serialize(HkArray *arr, FILE *stream)
{
  fwrite(&arr->capacity, sizeof(arr->capacity), 1, stream);
  fwrite(&arr->length, sizeof(arr->length), 1, stream);
  HkValue *elements = arr->elements;
  for (int i = 0; i < arr->length; ++i)
    hk_value_serialize(elements[i], stream);
}

HkArray *hk_array_deserialize(FILE *stream)
{
  int capacity;
  int length;
  if (fread(&capacity, sizeof(capacity), 1, stream) != 1)
    return NULL;
  if (fread(&length, sizeof(length), 1, stream) != 1)
    return NULL;
  HkArray *arr = array_allocate(capacity);
  arr->length = length;
  for (int i = 0; i < length; ++i)
  {
    HkValue elem;
    if (!hk_value_deserialize(stream, &elem))
      return NULL;
    hk_value_incr_ref(elem);
    arr->elements[i] = elem;
  }
  return arr;
}
