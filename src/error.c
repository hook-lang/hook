//
// Hook Programming Language
// error.c
//

#include "error.h"
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
