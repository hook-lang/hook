//
// Hook Programming Language
// hook_range.c
//

#include "hook_range.h"
#include <stdlib.h>
#include "hook_memory.h"

typedef struct
{
  HK_ITERATOR_HEADER
  hk_range_t *iterable;
  int64_t current;
} range_iterator_t;

static void range_iterator_deinit(hk_iterator_t *it);
static bool range_iterator_is_valid(hk_iterator_t *it);
static hk_value_t range_iterator_get_current(hk_iterator_t *it);
static void range_iterator_next(hk_iterator_t *it);

static void range_iterator_deinit(hk_iterator_t *it)
{
  hk_range_release(((range_iterator_t *) it)->iterable);
}

static bool range_iterator_is_valid(hk_iterator_t *it)
{
  range_iterator_t *range_it = (range_iterator_t *) it;
  hk_range_t *range = range_it->iterable;
  if (range->step == 1)
    return range_it->current <= range->end;
  return range_it->current >= range->end;
}

static hk_value_t range_iterator_get_current(hk_iterator_t *it)
{
  range_iterator_t *range_it = (range_iterator_t *) it;
  return hk_float_value((double) range_it->current);
}

static void range_iterator_next(hk_iterator_t *it)
{
  range_iterator_t *range_it = (range_iterator_t *) it;
  hk_range_t *range = range_it->iterable;
  range_it->current += range->step;
}

hk_range_t *hk_range_new(int64_t start, int64_t end)
{
  hk_range_t *range = (hk_range_t *) hk_allocate(sizeof(*range));
  range->ref_count = 0;
  range->step = start < end ? 1 : -1;
  range->start = start;
  range->end = end;
  return range;
}

void hk_range_free(hk_range_t *range)
{
  free(range);
}

void hk_range_release(hk_range_t *range)
{
  hk_decr_ref(range);
  if (hk_is_unreachable(range))
    hk_range_free(range);
}

void hk_range_print(hk_range_t *range)
{
  printf("%lld..%lld", (long long) range->start, (long long) range->end);
}

bool hk_range_equal(hk_range_t *range1, hk_range_t *range2)
{
  return range1->start == range2->start
    && range1->end == range2->end;
}

int32_t hk_range_compare(hk_range_t *range1, hk_range_t *range2)
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

hk_iterator_t *hk_range_new_iterator(hk_range_t *range)
{
  range_iterator_t *it = (range_iterator_t *) hk_allocate(sizeof(*it));
  hk_iterator_init((hk_iterator_t *) it, &range_iterator_deinit,
    &range_iterator_is_valid, &range_iterator_get_current,
    &range_iterator_next);
  hk_incr_ref(range);
  it->iterable = range;
  it->current = range->start;
  return (hk_iterator_t *) it;
}
