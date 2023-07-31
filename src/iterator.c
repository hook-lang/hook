//
// The Hook Programming Language
// iterator.c
//

#include <hook/iterator.h>
#include <stdlib.h>

void hk_iterator_init(HkIterator *it, void (*deinit)(struct hk_iterator *),
  bool (*is_valid)(struct hk_iterator *), HkValue (*get_current)(struct hk_iterator *),
  struct hk_iterator *(*next)(struct hk_iterator *), void (*inplace_next)(struct hk_iterator *))
{
  it->ref_count = 0;
  it->deinit = deinit;
  it->is_valid = is_valid;
  it->get_current = get_current;
  it->next = next;
  it->inplace_next = inplace_next;
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
  return it->is_valid(it);
}

HkValue hk_iterator_get_current(HkIterator *it)
{
  return it->get_current(it);
}

HkIterator *hk_iterator_next(HkIterator *it)
{
  return it->next(it);
}

void hk_iterator_inplace_next(HkIterator *it)
{
  it->inplace_next(it);
}
