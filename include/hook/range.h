//
// The Hook Programming Language
// range.h
//

#ifndef HK_RANGE_H
#define HK_RANGE_H

#include <hook/value.h>
#include <hook/iterator.h>

typedef struct
{
  HK_OBJECT_HEADER
  int32_t step;
  int64_t start;
  int64_t end;
} hk_range_t;

hk_range_t *hk_range_new(int64_t start, int64_t end);
void hk_range_free(hk_range_t *range);
void hk_range_release(hk_range_t *range);
void hk_range_print(hk_range_t *range);
bool hk_range_equal(hk_range_t *range1, hk_range_t *range2);
int32_t hk_range_compare(hk_range_t *range1, hk_range_t *range2);
hk_iterator_t *hk_range_new_iterator(hk_range_t *range);

#endif // HK_RANGE_H
