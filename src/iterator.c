//
// iterator.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include <hook/iterator.h>
#include <hook/memory.h>

void hk_iterator_init(HkIterator *it, void (*deinit)(HkIterator *),
  bool (*isValid)(HkIterator *), HkValue (*getCurrent)(HkIterator *),
  HkIterator *(*next)(HkIterator *), void (*inplaceNext)(HkIterator *))
{
  it->refCount = 0;
  it->deinit = deinit;
  it->isValid = isValid;
  it->getCurrent = getCurrent;
  it->next = next;
  it->inplaceNext = inplaceNext;
}

void hk_iterator_free(HkIterator *it)
{
  if (it->deinit)
    it->deinit(it);
  hk_free(it);
}

void hk_iterator_release(HkIterator *it)
{
  hk_decr_ref(it);
  if (hk_is_unreachable(it))
    hk_iterator_free(it);
}

bool hk_iterator_is_valid(HkIterator *it)
{
  return it->isValid(it);
}

HkValue hk_iterator_get_current(HkIterator *it)
{
  return it->getCurrent(it);
}

HkIterator *hk_iterator_next(HkIterator *it)
{
  return it->next(it);
}

void hk_iterator_inplace_next(HkIterator *it)
{
  it->inplaceNext(it);
}
