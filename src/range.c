//
// Hook Programming Language
// range.c
//

#include "range.h"
#include <stdlib.h>
#include "memory.h"

typedef struct
{
  ITERATOR_HEADER
  range_t *iterable;
  long current;
} range_iterator_t;

static void range_iterator_deinit(iterator_t *it);
static bool range_iterator_is_valid(iterator_t *it);
static value_t range_iterator_get_current(iterator_t *it);
static void range_iterator_next(iterator_t *it);

static void range_iterator_deinit(iterator_t *it)
{
  range_t *range = ((range_iterator_t *) it)->iterable;
  DECR_REF(range);
  if (IS_UNREACHABLE(range))
    range_free(range);
}

static bool range_iterator_is_valid(iterator_t *it)
{
  range_iterator_t *range_it = (range_iterator_t *) it;
  range_t *range = range_it->iterable;
  if (range->step == 1)
    return range_it->current <= range->end;
  return range_it->current >= range->end;
}

static value_t range_iterator_get_current(iterator_t *it)
{
  range_iterator_t *range_it = (range_iterator_t *) it;
  return NUMBER_VALUE((double) range_it->current);
}

static void range_iterator_next(iterator_t *it)
{
  range_iterator_t *range_it = (range_iterator_t *) it;
  range_t *range = range_it->iterable;
  range_it->current += range->step;
}

range_t *range_new(long start, long end)
{
  range_t *range = (range_t *) allocate(sizeof(*range));
  range->ref_count = 0;
  range->step = start < end ? 1 : -1;
  range->start = start;
  range->end = end;
  return range;
}

void range_free(range_t *range)
{
  free(range);
}

void range_print(range_t *range)
{
  printf("%ld..%ld", range->start, range->end);
}

bool range_equal(range_t *range1, range_t *range2)
{
  return range1->start == range2->start
    && range1->end == range2->end;
}

int range_compare(range_t *range1, range_t *range2)
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

iterator_t *range_new_iterator(range_t *range)
{
  range_iterator_t *it = (range_iterator_t *) allocate(sizeof(*it));
  iterator_init((iterator_t *) it, &range_iterator_deinit,
    &range_iterator_is_valid, &range_iterator_get_current,
    &range_iterator_next);
  INCR_REF(range);
  it->iterable = range;
  it->current = range->start;
  return (iterator_t *) it;
}
