//
// Hook Programming Language
// hook_iterator.h
//

#ifndef HOOK_ITERATOR_H
#define HOOK_ITERATOR_H

#include "hook_value.h"

#define HK_ITERATOR_HEADER HK_OBJECT_HEADER \
                           void (*deinit)(struct hk_iterator *); \
                           bool (*is_valid)(struct hk_iterator *); \
                           hk_value_t (*get_current)(struct hk_iterator *); \
                           void (*next)(struct hk_iterator *);

typedef struct hk_iterator
{
  HK_ITERATOR_HEADER
} hk_iterator_t;

void hk_iterator_init(hk_iterator_t *it, void (*deinit)(struct hk_iterator *),
  bool (*is_valid)(struct hk_iterator *), hk_value_t (*get_current)(struct hk_iterator *),
  void (*next)(struct hk_iterator *));
void hk_iterator_free(hk_iterator_t *it);
bool hk_iterator_is_valid(hk_iterator_t *it);
hk_value_t hk_iterator_get_current(hk_iterator_t *it);
void hk_iterator_next(hk_iterator_t *it);

#endif // HOOK_ITERATOR_H
