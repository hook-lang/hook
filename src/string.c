//
// The Hook Programming Language
// string.c
//

#include <hook/string.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <hook/memory.h>
#include <hook/utils.h>

static inline HkString *string_allocate(int minCapacity);
static inline void add_char(HkString *str, char c);
static inline uint32_t hash(int length, char *chars);

static inline HkString *string_allocate(int minCapacity)
{
  HkString *str = (HkString *) hk_allocate(sizeof(*str));
  ++minCapacity;
  int capacity = minCapacity < HK_STRING_MIN_CAPACITY ? HK_STRING_MIN_CAPACITY : minCapacity;
  capacity = hk_power_of_two_ceil(capacity);
  str->ref_count = 0;
  str->capacity = capacity;
  str->chars = (char *) hk_allocate(capacity);
  str->hash = -1;
  return str;
}

static inline void add_char(HkString *str, char c)
{
  hk_string_ensure_capacity(str, str->length + 1);
  str->chars[str->length] = c;
}

static inline uint32_t hash(int length, char *chars)
{
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++)
  {
    hash ^= chars[i];
    hash *= 16777619;
  }
  return hash;
}

HkString *hk_string_new(void)
{
  return hk_string_new_with_capacity(0);
}

HkString *hk_string_new_with_capacity(int minCapacity)
{
  HkString *str = string_allocate(minCapacity);
  str->length = 0;
  str->chars[0] = '\0';
  return str;
}

HkString *hk_string_from_chars(int length, const char *chars)
{
  if (length < 0)
    length = (int) strnlen(chars, INT_MAX);
  HkString *str = string_allocate(length);
  str->length = length;
  memcpy(str->chars, chars, length);
  str->chars[length] = '\0';
  return str;
}

HkString *hk_string_from_stream(FILE *stream, const char terminal)
{
  HkString *str = string_allocate(0);
  str->length = 0;
  for (;;)
  {
    int c = fgetc(stream);
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

void hk_string_ensure_capacity(HkString *str, int minCapacity)
{
  if (minCapacity <= str->capacity)
    return;
  int capacity = hk_power_of_two_ceil(minCapacity);
  str->capacity = capacity;
  str->chars = (char *) hk_reallocate(str->chars, capacity);
}

void hk_string_free(HkString *str)
{
  free(str->chars);
  free(str);
}

void hk_string_release(HkString *str)
{
  hk_decr_ref(str);
  if (hk_is_unreachable(str))
    hk_string_free(str);
}

HkString *hk_string_concat(HkString *str1, HkString *str2)
{
  int length = str1->length + str2->length;
  HkString *result = string_allocate(length);
  memcpy(result->chars, str1->chars, str1->length);
  memcpy(&result->chars[str1->length], str2->chars, str2->length);
  result->length = length;
  result->chars[length] = '\0';
  return result;
}

void hk_string_inplace_concat_char(HkString *dest, char c)
{
  int length = dest->length;
  hk_string_ensure_capacity(dest, length + 2);
  dest->chars[length] = c;
  dest->chars[length + 1] = '\0';
  dest->length += 1;
}

void hk_string_inplace_concat_chars(HkString *dest, int length, const char *chars)
{
  if (length < 0)
    length = (int) strnlen(chars, INT_MAX);
  int new_length = dest->length + length;
  hk_string_ensure_capacity(dest, new_length + 1);
  memcpy(&dest->chars[dest->length], chars, length);
  dest->length = new_length;
  dest->chars[new_length] = '\0';
  dest->hash = -1;
}

void hk_string_inplace_concat(HkString *dest, HkString *src)
{
  int length = dest->length + src->length;
  hk_string_ensure_capacity(dest, length + 1);
  memcpy(&dest->chars[dest->length], src->chars, src->length);
  dest->length = length;
  dest->chars[length] = '\0';
  dest->hash = -1;
}

void hk_string_print(HkString *str, bool quoted)
{
  printf(quoted ? "\"%.*s\"" : "%.*s", str->length, str->chars);
}

uint32_t hk_string_hash(HkString *str)
{
  if (str->hash == -1)
    str->hash = hash(str->length, str->chars);
  return (uint32_t) str->hash;
}

bool hk_string_equal(HkString *str1, HkString *str2)
{
  return str1 == str2 || (str1->length == str2->length
    && !memcmp(str1->chars, str2->chars, str1->length));
}

int hk_string_compare(HkString *str1, HkString *str2)
{
  int result = strcmp(str1->chars, str2->chars);
  return result > 0 ? 1 : (result < 0 ? -1 : 0);
}

HkString *hk_string_lower(HkString *str)
{
  int length = str->length;
  HkString *result = string_allocate(length);
  result->length = length;
  for (int i = 0; i < length; ++i)
    result->chars[i] = (char) tolower(str->chars[i]);
  result->chars[length] = '\0';
  return result;
}

HkString *hk_string_upper(HkString *str)
{
  int length = str->length;
  HkString *result = string_allocate(length);
  result->length = length;
  for (int i = 0; i < length; ++i)
    result->chars[i] = (char) toupper(str->chars[i]);
  result->chars[length] = '\0';
  return result;
}

bool hk_string_trim(HkString *str, HkString **result)
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
  HkString *_result = string_allocate(h - l + 1);
  int j = 0;
  for (int i = l; i <= h; ++i)
    _result->chars[j++] = str->chars[i];
  _result->chars[j] = '\0';
  _result->length = j;
  *result = _result;
  return true;
}

bool hk_string_starts_with(HkString *str1, HkString *str2)
{
  if (!str1->length || !str2->length || str1->length < str2->length)
    return false;
  return !memcmp(str1->chars, str2->chars, str2->length);
}

bool hk_string_ends_with(HkString *str1, HkString *str2)
{
  if (!str1->length || !str2->length || str1->length < str2->length)
    return false;
  return !memcmp(&str1->chars[str1->length - str2->length], str2->chars, str2->length);
}

HkString *hk_string_reverse(HkString *str)
{
  int length = str->length;
  HkString *result = string_allocate(length);
  result->length = length;
  for (int i = 0; i < length; ++i)
    result->chars[i] = str->chars[length - i - 1];
  result->chars[length] = '\0';
  return result;
}

void hk_string_serialize(HkString *str, FILE *stream)
{
  fwrite(&str->capacity, sizeof(str->capacity), 1, stream);
  fwrite(&str->length, sizeof(str->length), 1, stream);
  fwrite(str->chars, str->length + 1, 1, stream);
  fwrite(&str->hash, sizeof(str->hash), 1, stream);
}

HkString *hk_string_deserialize(FILE *stream)
{
  int capacity;
  int length;
  if (fread(&capacity, sizeof(capacity), 1, stream) != 1)
    return NULL;
  if (fread(&length, sizeof(length), 1, stream) != 1)
    return NULL;
  HkString *str = string_allocate(capacity);
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
