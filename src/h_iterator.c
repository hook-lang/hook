//
// Hook Programming Language
// h_iterator.c
//

#include "h_iterator.h"
#include <stdlib.h>

void iterator_init(iterator_t *it, void (*deinit)(struct iterator *),
  bool (*is_valid)(struct iterator *), value_t (*get_current)(struct iterator *),
  void (*next)(struct iterator *))
{
  it->ref_count = 0;
  it->deinit = deinit;
  it->is_valid = is_valid;
  it->get_current = get_current;
  it->next = next;
}

void iterator_free(iterator_t *it)
{
  if (it->deinit)
    it->deinit(it);
  free(it);
}

bool iterator_is_valid(iterator_t *it)
{
  return it->is_valid(it);
}

value_t iterator_get_current(iterator_t *it)
{
  return it->get_current(it);
}

void iterator_next(iterator_t *it)
{
  it->next(it);
}
