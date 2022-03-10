//
// Hook Programming Language
// hook_iterator.h
//

#ifndef HOOK_ITERATOR_H
#define HOOK_ITERATOR_H

#include "hook_value.h"

#define ITERATOR_HEADER OBJECT_HEADER \
                        void (*deinit)(struct iterator *); \
                        bool (*is_valid)(struct iterator *); \
                        value_t (*get_current)(struct iterator *); \
                        void (*next)(struct iterator *);

typedef struct iterator
{
  ITERATOR_HEADER
} iterator_t;

void iterator_init(iterator_t *it, void (*deinit)(struct iterator *),
  bool (*is_valid)(struct iterator *), value_t (*get_current)(struct iterator *),
  void (*next)(struct iterator *));
void iterator_free(iterator_t *it);
bool iterator_is_valid(iterator_t *it);
value_t iterator_get_current(iterator_t *it);
void iterator_next(iterator_t *it);

#endif // HOOK_ITERATOR_H
