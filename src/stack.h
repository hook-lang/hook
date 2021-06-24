//
// Hook Programming Language
// stack.h
//

#ifndef STACK_H
#define STACK_H

#include "value.h"

#define STACK_DEFAULT_CAPACITY 256

typedef struct
{
  int capacity;
  int end;
  value_t *slots;
  int index;
  char pad[4];
} stack_t;

void stack_init(stack_t *stk, int min_capacity);
void stack_free(stack_t *stk);
void stack_push_double(stack_t *stk, double data);
value_t stack_pop(stack_t *stk);

#endif
