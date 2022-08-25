//
// The Hook Programming Language
// hk_string.c
//

#include "hk_string.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "hk_utils.h"
#include "hk_memory.h"

static inline hk_string_t *string_allocate(int32_t min_capacity);
static inline void add_char(hk_string_t *str, char c);
static inline uint32_t hash(int32_t length, char *chars);

static inline hk_string_t *string_allocate(int32_t min_capacity)
{
  hk_string_t *str = (hk_string_t *) hk_allocate(sizeof(*str));
  ++min_capacity;
  int32_t capacity = min_capacity < HK_STRING_MIN_CAPACITY ? HK_STRING_MIN_CAPACITY : min_capacity;
  capacity = hk_power_of_two_ceil(capacity);
  str->ref_count = 0;
  str->capacity = capacity;
  str->chars = (char *) hk_allocate(capacity);
  str->hash = -1;
  return str;
}

static inline void add_char(hk_string_t *str, char c)
{
  hk_string_ensure_capacity(str, str->length + 1);
  str->chars[str->length] = c;
}

static inline uint32_t hash(int32_t length, char *chars)
{
  uint32_t hash = 2166136261u;
  for (int32_t i = 0; i < length; i++)
  {
    hash ^= chars[i];
    hash *= 16777619;
  }
  return hash;
}

hk_string_t *hk_string_new(void)
{
  return hk_string_new_with_capacity(0);
}

hk_string_t *hk_string_new_with_capacity(int32_t min_capacity)
{
  hk_string_t *str = string_allocate(min_capacity);
  str->length = 0;
  str->chars[0] = '\0';
  return str;
}

hk_string_t *hk_string_from_chars(int32_t length, const char *chars)
{
  if (length < 0)
    length = (int32_t) strnlen(chars, INT_MAX);
  hk_string_t *str = string_allocate(length);
  str->length = length;
  memcpy(str->chars, chars, length);
  str->chars[length] = '\0';
  return str;
}

hk_string_t *hk_string_from_stream(FILE *stream, const char terminal)
{
  hk_string_t *str = string_allocate(0);
  str->length = 0;
  for (;;)
  {
    int32_t c = fgetc(stream);
    if (c == EOF)
    {
      if (feof(stream))
        break;
      hk_assert(!ferror(stream), "unexpected error on fgetc()");
    }
    if (c == terminal)
      break;
    add_char(str, (char) c);
    ++str->length;
  }
  add_char(str, '\0');
  return str;
}

void hk_string_ensure_capacity(hk_string_t *str, int32_t min_capacity)
{
  if (min_capacity <= str->capacity)
    return;
  int32_t capacity = hk_power_of_two_ceil(min_capacity);
  str->capacity = capacity;
  str->chars = (char *) hk_reallocate(str->chars, capacity);
}

void hk_string_free(hk_string_t *str)
{
  free(str->chars);
  free(str);
}

void hk_string_release(hk_string_t *str)
{
  hk_decr_ref(str);
  if (hk_is_unreachable(str))
    hk_string_free(str);
}

hk_string_t *hk_string_concat(hk_string_t *str1, hk_string_t *str2)
{
  int32_t length = str1->length + str2->length;
  hk_string_t *result = string_allocate(length);
  memcpy(result->chars, str1->chars, str1->length);
  memcpy(&result->chars[str1->length], str2->chars, str2->length);
  result->length = length;
  result->chars[length] = '\0';
  return result;
}

void hk_string_inplace_concat_chars(hk_string_t *dest, int32_t length, const char *chars)
{
  if (length < 0)
    length = (int32_t) strnlen(chars, INT_MAX);
  int32_t new_length = dest->length + length;
  hk_string_ensure_capacity(dest, new_length + 1);
  memcpy(&dest->chars[dest->length], chars, length);
  dest->length = new_length;
  dest->chars[new_length] = '\0';
  dest->hash = -1;
}

void hk_string_inplace_concat(hk_string_t *dest, hk_string_t *src)
{
  int32_t length = dest->length + src->length;
  hk_string_ensure_capacity(dest, length + 1);
  memcpy(&dest->chars[dest->length], src->chars, src->length);
  dest->length = length;
  dest->chars[length] = '\0';
  dest->hash = -1;
}

void hk_string_print(hk_string_t *str, bool quoted)
{
  printf(quoted ? "\"%.*s\"" : "%.*s", str->length, str->chars);
}

uint32_t hk_string_hash(hk_string_t *str)
{
  if (str->hash == -1)
    str->hash = hash(str->length, str->chars);
  return (uint32_t) str->hash;
}

bool hk_string_equal(hk_string_t *str1, hk_string_t *str2)
{
  return str1 == str2 || (str1->length == str2->length
    && !memcmp(str1->chars, str2->chars, str1->length));
}

int32_t hk_string_compare(hk_string_t *str1, hk_string_t *str2)
{
  int32_t result = strcmp(str1->chars, str2->chars);
  return result > 0 ? 1 : (result < 0 ? -1 : 0);
}

hk_string_t *hk_string_lower(hk_string_t *str)
{
  int32_t length = str->length;
  hk_string_t *result = string_allocate(length);
  result->length = length;
  for (int32_t i = 0; i < length; ++i)
    result->chars[i] = (char) tolower(str->chars[i]);
  result->chars[length] = '\0';
  return result;
}

hk_string_t *hk_string_upper(hk_string_t *str)
{
  int32_t length = str->length;
  hk_string_t *result = string_allocate(length);
  result->length = length;
  for (int32_t i = 0; i < length; ++i)
    result->chars[i] = (char) toupper(str->chars[i]);
  result->chars[length] = '\0';
  return result;
}

bool hk_string_trim(hk_string_t *str, hk_string_t **result)
{
  int32_t length = str->length;
  if (!length)
    return false;
  int32_t l = 0;
  while (isspace(str->chars[l]))
    ++l;
  int32_t high = length - 1;
  int32_t h = high;
  while (h > l && isspace(str->chars[h]))
    --h;
  if (!l && h == high)
    return false;
  hk_string_t *_result = string_allocate(h - l + 1);
  int32_t j = 0;
  for (int32_t i = l; i <= h; ++i)
    _result->chars[j++] = str->chars[i];
  _result->chars[j] = '\0';
  _result->length = j;
  *result = _result;
  return true;
}

bool hk_string_starts_with(hk_string_t *str1, hk_string_t *str2)
{
  if (!str1->length || !str2->length || str1->length < str2->length)
    return false;
  return !memcmp(str1->chars, str2->chars, str2->length);
}

bool hk_string_ends_with(hk_string_t *str1, hk_string_t *str2)
{
  if (!str1->length || !str2->length || str1->length < str2->length)
    return false;
  return !memcmp(&str1->chars[str1->length - str2->length], str2->chars, str2->length);
}

hk_string_t *hk_string_reverse(hk_string_t *str)
{
  int32_t length = str->length;
  hk_string_t *result = string_allocate(length);
  result->length = length;
  for (int32_t i = 0; i < length; ++i)
    result->chars[i] = str->chars[length - i - 1];
  result->chars[length] = '\0';
  return result;
}

void hk_string_serialize(hk_string_t *str, FILE *stream)
{
  fwrite(&str->capacity, sizeof(str->capacity), 1, stream);
  fwrite(&str->length, sizeof(str->length), 1, stream);
  fwrite(str->chars, str->length + 1, 1, stream);
  fwrite(&str->hash, sizeof(str->hash), 1, stream);
}

hk_string_t *hk_string_deserialize(FILE *stream)
{
  int32_t capacity;
  int32_t length;
  if (fread(&capacity, sizeof(capacity), 1, stream) != 1)
    return NULL;
  if (fread(&length, sizeof(length), 1, stream) != 1)
    return NULL;
  hk_string_t *str = string_allocate(capacity);
  str->length = length;
  str->chars[length] = '\0';
  if (fread(str->chars, length + 1, 1, stream) != 1)
  {
    hk_string_free(str);
    return NULL;
  }
  if (fread(&str->hash, sizeof(str->hash), 1, stream) != 1)
  {
    hk_string_free(str);
    return NULL;
  }
  return str;
}
