//
// The Hook Programming Language
// iterable.c
//

#include <hook/iterable.h>
#include <hook/array.h>
#include <hook/range.h>

HkIterator *hk_new_iterator(HkValue val)
{
  if (!hk_is_iterable(val))
    return NULL;
  if (hk_is_range(val))
    return hk_range_new_iterator(hk_as_range(val));
  return hk_array_new_iterator(hk_as_array(val));
}
