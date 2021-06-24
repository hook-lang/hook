//
// Hook Programming Language
// string.c
//

#include "string.h"
#include <stdlib.h>
#include <string.h>
#include "memory.h"

static inline string_t *string_allocate(int min_capacity);
static inline void resize(string_t *str);
static inline void append_char(string_t *str, char c);

static inline string_t *string_allocate(int min_capacity)
{
  string_t *str = (string_t *) allocate(sizeof(*str));
  int capacity = STRING_MIN_CAPACITY;
  while (capacity < min_capacity)
    capacity <<= 1;
  char *chars = (char *) allocate(capacity);
  str->capacity = capacity;
  str->chars = chars;
  return str;
}

static inline void resize(string_t *str)
{
  if (str->length < str->capacity)
    return;
  int capacity = str->capacity << 1;
  char *chars = (char *) reallocate(str->chars, capacity);
  str->capacity = capacity;
  str->chars = chars;
}

static inline void append_char(string_t *str, char c)
{
  resize(str);
  str->chars[str->length] = c;
  ++str->length;
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

void string_free(string_t *str)
{
  free(str->chars);
  free(str);
}
