//
// Hook Programming Language
// h_error.c
//

#include "h_error.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void fatal_error(const char *fmt, ...)
{
  fprintf(stderr, "fatal error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
  exit(EXIT_FAILURE);
}

void runtime_error(const char *fmt, ...)
{
  fprintf(stderr, "runtime error: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}
