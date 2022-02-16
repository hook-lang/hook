//
// Hook Programming Language
// range.c
//

#include "range.h"
#include <stdlib.h>
#include "memory.h"

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
