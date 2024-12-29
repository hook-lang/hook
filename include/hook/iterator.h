//
// iterator.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_ITERATOR_H
#define HK_ITERATOR_H

#include "value.h"

#define HK_ITERATOR_HEADER HK_OBJECT_HEADER \
                           void (*deinit)(struct HkIterator *); \
                           bool (*isValid)(struct HkIterator *); \
                           HkValue (*getCurrent)(struct HkIterator *); \
                           struct HkIterator *(*next)(struct HkIterator *); \
                           void (*inplaceNext)(struct HkIterator *);

typedef struct HkIterator
{
  HK_ITERATOR_HEADER
} HkIterator;

void hk_iterator_init(HkIterator *it, void (*deinit)(HkIterator *),
  bool (*isValid)(HkIterator *), HkValue (*getCurrent)(HkIterator *),
  HkIterator *(*next)(HkIterator *), void (*inplaceNext)(HkIterator *));
void hk_iterator_free(HkIterator *it);
void hk_iterator_release(HkIterator *it);
bool hk_iterator_is_valid(HkIterator *it);
HkValue hk_iterator_get_current(HkIterator *it);
HkIterator *hk_iterator_next(HkIterator *it);
void hk_iterator_inplace_next(HkIterator *it);

#endif // HK_ITERATOR_H
