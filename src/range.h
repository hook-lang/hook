//
// Hook Programming Language
// range.h
//

#ifndef RANGE_H
#define RANGE_H

#include "value.h"

typedef struct
{
  OBJECT_HEADER
  int step;
  long start;
  long end;
} range_t;

range_t *range_new(long start, long end);
void range_free(range_t *range);
void range_print(range_t *range);
bool range_equal(range_t *range1, range_t *range2);
int range_compare(range_t *range1, range_t *range2);

#endif // RANGE_H
