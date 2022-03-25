//
// Hook Programming Language
// hook_iterable.c
//

#include "hook_iterable.h"
#include "hook_range.h"
#include "hook_array.h"

hk_iterator_t *hk_new_iterator(hk_value_t val)
{
  if (!hk_is_iterable(val))
    return NULL;
  if (hk_is_range(val))
    return hk_range_new_iterator(hk_as_range(val));
  return hk_array_new_iterator(hk_as_array(val));
}
