//
// Hook Programming Language
// hook_utils.h
//

#ifndef HOOK_UTILS_H
#define HOOK_UTILS_H

#include <stdint.h>

#define hk_assert(cond, msg) do \
  { \
    if (!(cond)) \
    { \
      fprintf(stderr, "assertion failed: %s\n  at %s() in %s:%d", \
        (msg), __func__, __FILE__, __LINE__); \
      exit(EXIT_FAILURE); \
    } \
  } while(0)

int32_t hk_power_of_two_ceil(int32_t n);
void hk_ensure_path(const char *filename);

#endif // HOOK_UTILS_H
