//
// Hook Programming Language
// string.c
//

#include "string.h"
#include <stdlib.h>
#include <string.h>
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
    length = (int) strlen(chars);
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
  int c = fgetc(fp);
  while (c != EOF)
  {
    append_char(str, (char) c);
    c = fgetc(fp);
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
  fread(str->chars, 1, length, fp);
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
}

bool string_equal(string_t *str1, string_t *str2)
{
  return str1->length == str2->length
    && !memcmp(str1->chars, str2->chars, str1->length);
}

int string_compare(string_t *str1, string_t *str2)
{
  return strcmp(str1->chars, str2->chars);
}
