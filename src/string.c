//
// Hook Programming Language
// string.c
//

#include "string.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "hash.h"
#include "common.h"
#include "memory.h"
#include "error.h"

static inline string_t *string_allocate(int min_capacity);
static inline void resize(string_t *str, int min_capacity);
static inline void append_char(string_t *str, char c);
static inline FILE *open_file(const char *filename, const char *mode);

static inline string_t *string_allocate(int min_capacity)
{
  string_t *str = (string_t *) allocate(sizeof(*str));
  int capacity = STRING_MIN_CAPACITY;
  while (capacity < min_capacity)
    capacity <<= 1;
  str->ref_count = 0;
  str->capacity = capacity;
  str->chars = (char *) allocate(capacity);
  str->hash = -1;
  return str;
}

static inline void resize(string_t *str, int min_capacity)
{
  if (min_capacity <= str->capacity)
    return;
  int capacity = str->capacity;
  while (capacity < min_capacity)
    capacity <<= 1;
  str->capacity = capacity;
  str->chars = (char *) reallocate(str->chars, capacity);
}

static inline void append_char(string_t *str, char c)
{
  resize(str, str->length + 1);
  str->chars[str->length] = c;
  ++str->length;
}

static inline FILE *open_file(const char *filename, const char *mode)
{
  FILE *fp = fopen(filename, mode);
  if (!fp)
    fatal_error("unable to open file '%s'", filename);
  return fp;
}

string_t *string_from_chars(int length, const char *chars)
{
  if (length < 0)
    length = (int) strnlen(chars, INT_MAX);
  string_t *str = string_allocate(length + 1);
  str->length = length;
  memcpy(str->chars, chars, length);
  str->chars[length] = '\0';
  return str;
}

string_t *string_from_stream(FILE *fp)
{
  string_t *str = string_allocate(0);
  str->length = 0;
  for (;;)
  {
    int c = fgetc(fp);
    if (c == EOF)
    {
      if (feof(fp))
        break;
      ASSERT(!ferror(fp), "unexpected error reading stream");
    }
    append_char(str, (char) c);
  }
  append_char(str, '\0');
  return str;
}

string_t *string_from_file(const char *filename)
{
  FILE *fp = open_file(filename, "rb");
  fseek(fp, 0L, SEEK_END);
  int length = ftell(fp);
  rewind(fp);
  string_t *str = string_allocate(length + 1);
  str->length = length;
  ASSERT(fread(str->chars, length, 1, fp) == 1, "unexpected error reading stream");
  str->chars[length] = '\0';
  fclose(fp);
  return str;
}

void string_free(string_t *str)
{
  free(str->chars);
  free(str);
}

string_t *string_concat(string_t *str1, string_t *str2)
{
  int length = str1->length + str2->length;
  string_t *result = string_allocate(length + 1);
  memcpy(result->chars, str1->chars, str1->length);
  memcpy(&result->chars[str1->length], str2->chars, str2->length);
  result->length = length;
  result->chars[length] = '\0';
  return result;
}

void string_inplace_concat(string_t *dest, string_t *src)
{
  int length = dest->length + src->length;
  resize(dest, length + 1);
  memcpy(&dest->chars[dest->length], src->chars, src->length);
  dest->length = length;
  dest->chars[length] = '\0';
  dest->hash = -1;
}

void string_print(string_t *str, bool quoted)
{
  printf(quoted ? "'%.*s'" : "%.*s", str->length, str->chars);
}

uint32_t string_hash(string_t *str)
{
  if (str->hash == -1)
    str->hash = hash(str->length, str->chars);
  return (uint32_t) str->hash;
}

bool string_equal(string_t *str1, string_t *str2)
{
  return str1 == str2 || (str1->length == str2->length
    && !memcmp(str1->chars, str2->chars, str1->length));
}

int string_compare(string_t *str1, string_t *str2)
{
  int result = strcmp(str1->chars, str2->chars);
  return result > 0 ? 1 : (result < 0 ? -1 : 0);
}

string_t *string_lower(string_t *str)
{
  int length = str->length;
  string_t *result = string_allocate(length + 1);
  result->length = length;
  for (int i = 0; i < length; i++)
    result->chars[i] = (char) tolower(str->chars[i]);
  result->chars[length] = '\0';
  return result;
}

string_t *string_upper(string_t *str)
{
  int length = str->length;
  string_t *result = string_allocate(length + 1);
  result->length = length;
  for (int i = 0; i < length; i++)
    result->chars[i] = (char) toupper(str->chars[i]);
  result->chars[length] = '\0';
  return result;
}
