//
// Hook Programming Language
// string.h
//

#ifndef STRING_H
#define STRING_H

#include <stdio.h>
#include "value.h"

#define STRING_MIN_CAPACITY 8

typedef struct
{
  OBJECT_HEADER
  int capacity;
  int length;
  char *chars;
} string_t;

string_t *string_from_chars(int length, const char *chars);
string_t *string_from_stream(FILE *fp);
string_t *string_from_file(const char *filename);
void string_free(string_t *str);
string_t *string_concat(string_t *str1, string_t *str2);
void string_inplace_concat(string_t *dest, string_t *src);
bool string_equal(string_t *str1, string_t *str2);
int string_compare(string_t *str1, string_t *str2);
string_t *string_lower(string_t *str);
string_t *string_upper(string_t *str);

#endif
