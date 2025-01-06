// 
// stack.h
// 
// Copyright 2021 The Hook Programming Language Authors.
// 
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_STACK_H
#define HK_STACK_H

#include "memory.h"

#define HkStack(T) \
  struct { \
    T *base; \
    T *top; \
    T *limit; \
  }

#define hk_stack_init(stk, sz) \
  do { \
    size_t _size = sizeof(*(stk)->base) * (sz); \
    void *base = hk_allocate(_size); \
    (stk)->base = base; \
    (stk)->top = &(stk)->base[-1]; \
    (stk)->limit = &(stk)->base[(sz) - 1]; \
  } while (0)

#define hk_stack_deinit(stk) \
  do { \
    hk_free((stk)->base); \
  } while (0)

#define hk_stack_is_empty(stk) ((stk)->top == &(stk)->base[-1])

#define hk_stack_is_full(stk) ((stk)->top == (stk)->limit)

#define hk_stack_get(stk, i) ((stk)->top[- (i)])

#define hk_stack_set(stk, i, d) \
  do { \
    (stk)->top[- (i)] = (d); \
  } while (0)

#define hk_stack_push(stk, d) \
  do { \
    ++(stk)->top; \
    (stk)->top[0] = (d); \
  } while (0)

#define hk_stack_pop(stk) \
  do { \
    --(stk)->top; \
  } while (0)

#endif // HK_STACK_H
