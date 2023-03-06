//
// The Hook Programming Language
// iterator.c
//

#include <hook/iterator.h>
#include <stdlib.h>

void hk_iterator_init(hk_iterator_t *it, void (*deinit)(struct hk_iterator *),
  bool (*is_valid)(struct hk_iterator *), hk_value_t (*get_current)(struct hk_iterator *),
  struct hk_iterator *(*next)(struct hk_iterator *), void (*inplace_next)(struct hk_iterator *))
{
  it->ref_count = 0;
  it->deinit = deinit;
  it->is_valid = is_valid;
  it->get_current = get_current;
  it->next = next;
  it->inplace_next = inplace_next;
}

void hk_iterator_free(hk_iterator_t *it)
{
  if (it->deinit)
    it->deinit(it);
  free(it);
}

void hk_iterator_release(hk_iterator_t *it)
{
  hk_decr_ref(it);
  if (hk_is_unreachable(it))
    hk_iterator_free(it);
}

bool hk_iterator_is_valid(hk_iterator_t *it)
{
  return it->is_valid(it);
}

hk_value_t hk_iterator_get_current(hk_iterator_t *it)
{
  return it->get_current(it);
}

hk_iterator_t *hk_iterator_next(hk_iterator_t *it)
{
  return it->next(it);
}

void hk_iterator_inplace_next(hk_iterator_t *it)
{
  it->inplace_next(it);
}
