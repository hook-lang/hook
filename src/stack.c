//
// Hook Programming Language
// stack.c
//

#include "stack.h"
#include <stdlib.h>
#include "memory.h"
#include "error.h"

void stack_init(stack_t *stk, int min_capacity)
{
  int capacity = STACK_DEFAULT_CAPACITY;
  while (capacity < min_capacity)
    capacity <<= 1;
  value_t *slots = (value_t *) allocate(sizeof(*slots) * capacity);
  stk->capacity = capacity;
  stk->end = capacity - 1;
  stk->slots = slots;
  stk->index = -1;
}

void stack_free(stack_t *stk)
{
  free(stk->slots);
}

void stack_push_double(stack_t *stk, double data)
{
  if (stk->index == stk->end)
    fatal_error("stack overflow");
  ++stk->index;
  stk->slots[stk->index] = data;
}

value_t stack_pop(stack_t *stk)
{
  if (stk->index == -1)
    fatal_error("stack underflow");
  value_t val = stk->slots[stk->index];
  --stk->index;
  return val;
}
