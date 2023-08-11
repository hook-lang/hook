//
// The Hook Programming Language
// range.c
//

#include <hook/range.h>
#include <stdlib.h>
#include <hook/memory.h>

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
   RangeIterator *range_it = (RangeIterator *) hk_allocate(sizeof(*range_it));
  hk_iterator_init((HkIterator *) range_it, range_iterator_deinit,
    range_iterator_is_valid, range_iterator_get_current,
    range_iterator_next, range_iterator_inplace_next);
  hk_incr_ref(range);
  range_it->range = range;
  return range_it;
}

static void range_iterator_deinit(HkIterator *it)
{
  hk_range_release(((RangeIterator *) it)->range);
}

static bool range_iterator_is_valid(HkIterator *it)
{
  RangeIterator *range_it = (RangeIterator *) it;
  HkRange *range = range_it->range;
  if (range->step == 1)
    return range_it->current <= range->end;
  return range_it->current >= range->end;
}

static HkValue range_iterator_get_current(HkIterator *it)
{
  RangeIterator *range_it = (RangeIterator *) it;
  return hk_number_value((double) range_it->current);
}

static HkIterator *range_iterator_next(HkIterator *it)
{
  RangeIterator *range_it = (RangeIterator *) it;
  HkRange *range = range_it->range;
  RangeIterator *result = range_iterator_allocate(range);
  result->current = range_it->current + range->step;
  return (HkIterator *) result;
}

static void range_iterator_inplace_next(HkIterator *it)
{
  RangeIterator *range_it = (RangeIterator *) it;
  HkRange *range = range_it->range;
  range_it->current += range->step;
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
  free(range);
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
  RangeIterator *range_it = range_iterator_allocate(range);
  range_it->current = range->start;
  return (HkIterator *) range_it;
}
