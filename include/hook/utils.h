//
// utils.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_UTILS_H
#define HK_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#define HK_LOAD_MODULE_HANDLER_PREFIX "load_"

#ifdef _WIN32
  #define HK_LOAD_MODULE_HANDLER(n) void __declspec(dllexport) __stdcall load_##n(HkVM *vm)
#else
  #define HK_LOAD_MODULE_HANDLER(n) void load_##n(HkVM *vm)
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
char *hk_duplicate_cstring(const char *str);

#endif // HK_UTILS_H
