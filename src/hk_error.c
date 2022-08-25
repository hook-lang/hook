//
// The Hook Programming Language
// hk_error.c
//

#include "hk_error.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void hk_fatal_error(const char *fmt, ...)
{
  fprintf(stderr, "fatal error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
  exit(EXIT_FAILURE);
}

void hk_runtime_error(const char *fmt, ...)
{
  fprintf(stderr, "runtime error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}
