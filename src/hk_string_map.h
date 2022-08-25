//
// The Hook Programming Language
// hk_string_map.h
//

#ifndef HK_STRING_MAP_H
#define HK_STRING_MAP_H

#include "hk_string.h"

#define STRING_MAP_MIN_CAPACITY    (1 << 3)
#define STRING_MAP_MAX_LOAD_FACTOR 0.75

typedef struct
{
  hk_string_t *key;
  hk_value_t value;
} string_map_entry_t;

typedef struct
{
  int32_t capacity;
  int32_t mask;
  int32_t length;
  string_map_entry_t *entries;
} string_map_t;

void string_map_init(string_map_t *map, int32_t min_capacity);
void string_map_free(string_map_t *map);
string_map_entry_t *string_map_get_entry(string_map_t *map, hk_string_t *key);
void string_map_inplace_put(string_map_t *map, hk_string_t *key, hk_value_t value);

#endif // HK_STRING_MAP_H
