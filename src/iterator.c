//
// The Hook Programming Language
// iterator.c
//

#include <hook/iterator.h>
#include <stdlib.h>

void hk_iterator_init(HkIterator *it, void (*deinit)(struct hk_iterator *),
  bool (*isValid)(struct hk_iterator *), HkValue (*getCurrent)(struct hk_iterator *),
  struct hk_iterator *(*next)(struct hk_iterator *), void (*inplaceNext)(struct hk_iterator *))
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
  free(it);
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
