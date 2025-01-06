//
// range.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "hook/range.h"
#include "hook/memory.h"

typedef struct
{
  HK_ITERATOR_HEADER
  HkRange *range;
  int64_t current;
} RangeIterator;

static inline RangeIterator *range_iterator_allocate(HkRange *range);
static void range_iterator_deinit(HkIterator *it);
static bool range_iterator_is_valid(HkIterator *it);
static HkValue range_iterator_get_current(HkIterator *it);
static HkIterator *range_iterator_next(HkIterator *it);
static void range_iterator_inplace_next(HkIterator *it);

static inline RangeIterator *range_iterator_allocate(HkRange *range)
{
   RangeIterator *rangeIt = (RangeIterator *) hk_allocate(sizeof(*rangeIt));
   hk_iterator_init((HkIterator *) rangeIt, range_iterator_deinit,
   range_iterator_is_valid, range_iterator_get_current,
   range_iterator_next, range_iterator_inplace_next);
  hk_incr_ref(range);
  rangeIt->range = range;
  return rangeIt;
}

static void range_iterator_deinit(HkIterator *it)
{
  hk_range_release(((RangeIterator *) it)->range);
}

static bool range_iterator_is_valid(HkIterator *it)
{
  RangeIterator *rangeIt = (RangeIterator *) it;
  HkRange *range = rangeIt->range;
  if (range->step == 1)
    return rangeIt->current <= range->end;
  return rangeIt->current >= range->end;
}

static HkValue range_iterator_get_current(HkIterator *it)
{
  RangeIterator *rangeIt = (RangeIterator *) it;
  return hk_number_value((double) rangeIt->current);
}

static HkIterator *range_iterator_next(HkIterator *it)
{
  RangeIterator *rangeIt = (RangeIterator *) it;
  HkRange *range = rangeIt->range;
  RangeIterator *result = range_iterator_allocate(range);
  result->current = rangeIt->current + range->step;
  return (HkIterator *) result;
}

static void range_iterator_inplace_next(HkIterator *it)
{
  RangeIterator *rangeIt = (RangeIterator *) it;
  HkRange *range = rangeIt->range;
  rangeIt->current += range->step;
}

HkRange *hk_range_new(int64_t start, int64_t end)
{
  HkRange *range = (HkRange *) hk_allocate(sizeof(*range));
  range->refCount = 0;
  range->step = start < end ? 1 : -1;
  range->start = start;
  range->end = end;
  return range;
}

void hk_range_free(HkRange *range)
{
  hk_free(range);
}

void hk_range_release(HkRange *range)
{
  hk_decr_ref(range);
  if (hk_is_unreachable(range))
    hk_range_free(range);
}

void hk_range_print(HkRange *range)
{
  printf("%lld..%lld", (long long) range->start, (long long) range->end);
}

bool hk_range_equal(HkRange *range1, HkRange *range2)
{
  return range1->start == range2->start
    && range1->end == range2->end;
}

int hk_range_compare(HkRange *range1, HkRange *range2)
{
  if (range1->start < range2->start)
    return -1;
  if (range1->start > range2->start)
    return 1;
  if (range1->end < range2->end)
    return -1;
  if (range1->end > range2->end)
    return 1;
  return 0;
}

HkIterator *hk_range_new_iterator(HkRange *range)
{
  RangeIterator *rangeIt = range_iterator_allocate(range);
  rangeIt->current = range->start;
  return (HkIterator *) rangeIt;
}
