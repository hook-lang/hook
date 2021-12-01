//
// Hook Programming Language
// string.h
//

#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include "value.h"

#define STRING_MIN_CAPACITY (1 << 3)

typedef struct
{
  OBJECT_HEADER
  int capacity;
  int length;
  char *chars;
  int64_t hash;
} string_t;

string_t *string_allocate(int min_capacity);
string_t *string_new(int min_capacity);
string_t *string_from_chars(int length, const char *chars);
string_t *string_from_stream(FILE *stream, char terminal);
void string_free(string_t *str);
string_t *string_concat(string_t *str1, string_t *str2);
void string_inplace_concat_chars(string_t *dest, int length, const char *chars);
void string_inplace_concat(string_t *dest, string_t *src);
void string_print(string_t *str, bool quoted);
uint32_t string_hash(string_t *str);
bool string_equal(string_t *str1, string_t *str2);
int string_compare(string_t *str1, string_t *str2);
string_t *string_lower(string_t *str);
string_t *string_upper(string_t *str);
bool string_trim(string_t *str, string_t **result);
bool string_starts_with(string_t *str1, string_t *str2);
bool string_ends_with(string_t *str1, string_t *str2);
bool string_slice(string_t *str, int start, int stop, string_t **result);
void string_serialize(string_t *str, FILE *stream);
string_t *string_deserialize(FILE *stream);

#endif // STRING_H
