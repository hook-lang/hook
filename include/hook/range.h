//
// range.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_RANGE_H
#define HK_RANGE_H

#include "iterator.h"
#include "value.h"

typedef struct
{
  HK_OBJECT_HEADER
  int     step;
  int64_t start;
  int64_t end;
} HkRange;

HkRange *hk_range_new(int64_t start, int64_t end);
void hk_range_free(HkRange *range);
void hk_range_release(HkRange *range);
void hk_range_print(HkRange *range);
bool hk_range_equal(HkRange *range1, HkRange *range2);
int hk_range_compare(HkRange *range1, HkRange *range2);
HkIterator *hk_range_new_iterator(HkRange *range);

#endif // HK_RANGE_H
