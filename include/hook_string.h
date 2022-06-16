//
// Hook Programming Language
// hook_string.h
//

#ifndef HOOK_STRING_H
#define HOOK_STRING_H

#include "hook_value.h"

#define HK_STRING_MIN_CAPACITY (1 << 3)

typedef struct
{
  HK_OBJECT_HEADER
  int32_t capacity;
  int32_t length;
  char *chars;
  int64_t hash;
} hk_string_t;

hk_string_t *hk_string_new(void);
hk_string_t *hk_string_new_with_capacity(int32_t min_capacity);
hk_string_t *hk_string_from_chars(int32_t length, const char *chars);
hk_string_t *hk_string_from_stream(FILE *stream, const char terminal);
void hk_string_ensure_capacity(hk_string_t *str, int32_t min_capacity);
void hk_string_free(hk_string_t *str);
void hk_string_release(hk_string_t *str);
hk_string_t *hk_string_concat(hk_string_t *str1, hk_string_t *str2);
void hk_string_inplace_concat_chars(hk_string_t *dest, int32_t length, const char *chars);
void hk_string_inplace_concat(hk_string_t *dest, hk_string_t *src);
void hk_string_print(hk_string_t *str, bool quoted);
uint32_t hk_string_hash(hk_string_t *str);
bool hk_string_equal(hk_string_t *str1, hk_string_t *str2);
int32_t hk_string_compare(hk_string_t *str1, hk_string_t *str2);
hk_string_t *hk_string_lower(hk_string_t *str);
hk_string_t *hk_string_upper(hk_string_t *str);
bool hk_string_trim(hk_string_t *str, hk_string_t **result);
bool hk_string_starts_with(hk_string_t *str1, hk_string_t *str2);
bool hk_string_ends_with(hk_string_t *str1, hk_string_t *str2);
hk_string_t *hk_string_reverse(hk_string_t *str);
void hk_string_serialize(hk_string_t *str, FILE *stream);
hk_string_t *hk_string_deserialize(FILE *stream);

#endif // HOOK_STRING_H
