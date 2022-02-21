//
// Hook Programming Language
// h_string.c
//

#include "h_string.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "h_hash.h"
#include "h_common.h"
#include "h_memory.h"

static inline void resize(string_t *str, int min_capacity);
static inline void add_char(string_t *str, char c);

static inline void resize(string_t *str, int min_capacity)
{
  if (min_capacity <= str->capacity)
    return;
  int capacity = nearest_power_of_two(str->capacity, min_capacity);
  str->capacity = capacity;
  str->chars = (char *) reallocate(str->chars, capacity);
}

static inline void add_char(string_t *str, char c)
{
  resize(str, str->length + 1);
  str->chars[str->length] = c;
}

string_t *string_allocate(int min_capacity)
{
  ++min_capacity;
  string_t *str = (string_t *) allocate(sizeof(*str));
  int capacity = nearest_power_of_two(STRING_MIN_CAPACITY, min_capacity);
  str->ref_count = 0;
  str->capacity = capacity;
  str->chars = (char *) allocate(capacity);
  str->hash = -1;
  return str;
}

string_t *string_new(int min_capacity)
{
  string_t *str = string_allocate(min_capacity);
  str->length = 0;
  str->chars[0] = '\0';
  return str;
}

string_t *string_from_chars(int length, const char *chars)
{
  if (length < 0)
    length = (int) strnlen(chars, INT_MAX);
  string_t *str = string_allocate(length);
  str->length = length;
  memcpy(str->chars, chars, length);
  str->chars[length] = '\0';
  return str;
}

string_t *string_from_stream(FILE *stream, const char terminal)
{
  string_t *str = string_allocate(0);
  str->length = 0;
  for (;;)
  {
    int c = fgetc(stream);
    if (c == EOF)
    {
      if (feof(stream))
        break;
      ASSERT(!ferror(stream), "unexpected error on fgetc()");
    }
    if (c == terminal)
      break;
    add_char(str, (char) c);
    ++str->length;
  }
  add_char(str, '\0');
  return str;
}

void string_free(string_t *str)
{
  free(str->chars);
  free(str);
}

void string_release(string_t *str)
{
  DECR_REF(str);
  if (IS_UNREACHABLE(str))
    string_free(str);
}

string_t *string_concat(string_t *str1, string_t *str2)
{
  int length = str1->length + str2->length;
  string_t *result = string_allocate(length);
  memcpy(result->chars, str1->chars, str1->length);
  memcpy(&result->chars[str1->length], str2->chars, str2->length);
  result->length = length;
  result->chars[length] = '\0';
  return result;
}

void string_inplace_concat_chars(string_t *dest, int length, const char *chars)
{
  if (length < 0)
    length = (int) strnlen(chars, INT_MAX);
  int new_length = dest->length + length;
  resize(dest, new_length + 1);
  memcpy(&dest->chars[dest->length], chars, length);
  dest->length = new_length;
  dest->chars[new_length] = '\0';
  dest->hash = -1;
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
  printf(quoted ? "\"%.*s\"" : "%.*s", str->length, str->chars);
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
  string_t *result = string_allocate(length);
  result->length = length;
  for (int i = 0; i < length; ++i)
    result->chars[i] = (char) tolower(str->chars[i]);
  result->chars[length] = '\0';
  return result;
}

string_t *string_upper(string_t *str)
{
  int length = str->length;
  string_t *result = string_allocate(length);
  result->length = length;
  for (int i = 0; i < length; ++i)
    result->chars[i] = (char) toupper(str->chars[i]);
  result->chars[length] = '\0';
  return result;
}

bool string_trim(string_t *str, string_t **result)
{
  int length = str->length;
  if (!length)
    return false;
  int l = 0;
  while (isspace(str->chars[l]))
    ++l;
  int high = length - 1;
  int h = high;
  while (h > l && isspace(str->chars[h]))
    --h;
  if (!l && h == high)
    return false;
  string_t *_result = string_allocate(h - l + 1);
  int j = 0;
  for (int i = l; i <= h; ++i)
    _result->chars[j++] = str->chars[i];
  _result->chars[j] = '\0';
  _result->length = j;
  *result = _result;
  return true;
}

bool string_starts_with(string_t *str1, string_t *str2)
{
  if (!str1->length || !str2->length || str1->length < str2->length)
    return false;
  return !memcmp(str1->chars, str2->chars, str2->length);
}

bool string_ends_with(string_t *str1, string_t *str2)
{
  if (!str1->length || !str2->length || str1->length < str2->length)
    return false;
  return !memcmp(&str1->chars[str1->length - str2->length], str2->chars, str2->length);
}

bool string_slice(string_t *str, int start, int stop, string_t **result)
{
  if (start < 1 && stop >= str->length)
    return false;
  int length = stop - start;
  length = length < 0 ? 0 : length;
  string_t *slice = string_allocate(length);
  slice->length = length;
  for (int i = start, j = 0; i < stop; ++i, ++j)
    slice->chars[j] = str->chars[i];
  slice->chars[length] = '\0';
  *result = slice;
  return true;
}

void string_serialize(string_t *str, FILE *stream)
{
  fwrite(&str->capacity, sizeof(str->capacity), 1, stream);
  fwrite(&str->length, sizeof(str->length), 1, stream);
  fwrite(str->chars, str->length + 1, 1, stream);
  fwrite(&str->hash, sizeof(str->hash), 1, stream);
}

string_t *string_deserialize(FILE *stream)
{
  int capacity;
  int length;
  if (fread(&capacity, sizeof(capacity), 1, stream) != 1)
    return NULL;
  if (fread(&length, sizeof(length), 1, stream) != 1)
    return NULL;
  string_t *str = string_allocate(capacity);
  str->length = length;
  str->chars[length] = '\0';
  if (fread(str->chars, length + 1, 1, stream) != 1)
  {
    string_free(str);
    return NULL;
  }
  if (fread(&str->hash, sizeof(str->hash), 1, stream) != 1)
  {
    string_free(str);
    return NULL;
  }
  return str;
}
