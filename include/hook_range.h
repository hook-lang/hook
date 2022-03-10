//
// Hook Programming Language
// hook_range.h
//

#ifndef HOOK_RANGE_H
#define HOOK_RANGE_H

#include "hook_value.h"
#include "hook_iterator.h"

typedef struct
{
  OBJECT_HEADER
  int step;
  long start;
  long end;
} range_t;

range_t *range_new(long start, long end);
void range_free(range_t *range);
void range_release(range_t *range);
void range_print(range_t *range);
bool range_equal(range_t *range1, range_t *range2);
int range_compare(range_t *range1, range_t *range2);
iterator_t *range_new_iterator(range_t *range);

#endif // HOOK_RANGE_H
