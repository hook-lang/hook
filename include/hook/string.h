//
// string.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef HK_STRING_H
#define HK_STRING_H

#include "array.h"

#define HK_STRING_MIN_CAPACITY (1 << 3)

#define hk_string_is_empty(s)    (!(s)->length)
#define hk_string_get_char(s, i) ((s)->chars[(i)])

#define hk_string_inplace_clear(s) do \
  { \
    (s)->length = 0; \
    (s)->hash = -1; \
  } while (0)

typedef struct
{
  HK_OBJECT_HEADER
  int     capacity;
  int     length;
  char    *chars;
  int64_t hash;
} HkString;

HkString *hk_string_new(void);
HkString *hk_string_new_with_capacity(int minCapacity);
HkString *hk_string_from_chars(int length, const char *chars);
HkString *hk_string_from_stream(FILE *stream, const char delim);
void hk_string_ensure_capacity(HkString *str, int minCapacity);
void hk_string_free(HkString *str);
void hk_string_release(HkString *str);
HkString *hk_string_copy(HkString *str);
HkString *hk_string_concat(HkString *str1, HkString *str2);
void hk_string_inplace_concat_char(HkString *dest, char c);
void hk_string_inplace_concat_chars(HkString *dest, int length, const char *chars);
void hk_string_inplace_concat(HkString *dest, HkString *src);
int hk_string_index_of_chars(HkString *str, int length, const char *chars);
int hk_string_index_of(HkString *str, HkString *sub);
HkString *hk_string_replace_all(HkString *str, HkString *sub1, HkString *sub2);
HkString *hk_string_slice(HkString *str, int start, int stop);
HkArray *hk_string_split(HkString *str, HkString *sep);
void hk_string_print(HkString *str, bool quoted);
uint32_t hk_string_hash(HkString *str);
bool hk_string_equal(HkString *str1, HkString *str2);
int hk_string_compare(HkString *str1, HkString *str2);
HkString *hk_string_lower(HkString *str);
HkString *hk_string_upper(HkString *str);
bool hk_string_trim(HkString *str, HkString **result);
bool hk_string_starts_with(HkString *str1, HkString *str2);
bool hk_string_ends_with(HkString *str1, HkString *str2);
HkString *hk_string_reverse(HkString *str);
void hk_string_serialize(HkString *str, FILE *stream);
HkString *hk_string_deserialize(FILE *stream);

#endif // HK_STRING_H
