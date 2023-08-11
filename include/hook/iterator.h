//
// The Hook Programming Language
// iterator.h
//

#ifndef HK_ITERATOR_H
#define HK_ITERATOR_H

#include <hook/value.h>

#define HK_ITERATOR_HEADER HK_OBJECT_HEADER \
                           void (*deinit)(struct hk_iterator *); \
                           bool (*isValid)(struct hk_iterator *); \
                           HkValue (*getCurrent)(struct hk_iterator *); \
                           struct hk_iterator *(*next)(struct hk_iterator *); \
                           void (*inplaceNext)(struct hk_iterator *);

typedef struct hk_iterator
{
  HK_ITERATOR_HEADER
} HkIterator;

void hk_iterator_init(HkIterator *it, void (*deinit)(struct hk_iterator *),
  bool (*isValid)(struct hk_iterator *), HkValue (*getCurrent)(struct hk_iterator *),
  struct hk_iterator *(*next)(struct hk_iterator *), void (*inplaceNext)(struct hk_iterator *));
void hk_iterator_free(HkIterator *it);
void hk_iterator_release(HkIterator *it);
bool hk_iterator_is_valid(HkIterator *it);
HkValue hk_iterator_get_current(HkIterator *it);
HkIterator *hk_iterator_next(HkIterator *it);
void hk_iterator_inplace_next(HkIterator *it);

#endif // HK_ITERATOR_H
