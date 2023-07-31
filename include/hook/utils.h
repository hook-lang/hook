//
// The Hook Programming Language
// utils.h
//

#ifndef HK_UTILS_H
#define HK_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#define HK_LOAD_FN_PREFIX "load_"

#ifdef _WIN32
  #define HK_LOAD_FN(n) int __declspec(dllexport) __stdcall load_##n(HkState *state)
#else
  #define HK_LOAD_FN(n) int load_##n(HkState *state)
#endif

#define hk_assert(cond, msg) do \
  { \
    if (!(cond)) \
    { \
      fprintf(stderr, "assertion failed: %s\n  at %s() in %s:%d\n", \
        (msg), __func__, __FILE__, __LINE__); \
      exit(EXIT_FAILURE); \
    } \
  } while(0)

int hk_power_of_two_ceil(int n);
void hk_ensure_path(const char *filename);
bool hk_long_from_chars(long *result, const char *chars);
bool hk_double_from_chars(double *result, const char *chars, bool strict);
void hk_copy_cstring(char *dest, const char *src, int max_len);

#endif // HK_UTILS_H
