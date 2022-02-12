//
// Hook Programming Language
// string_map.h
//

#ifndef STRING_MAP_H
#define STRING_MAP_H

#include "string.h"

#define STRING_MAP_MIN_CAPACITY    (1 << 3)
#define STRING_MAP_MAX_LOAD_FACTOR 0.75

typedef struct
{
  string_t *key;
  value_t value;
} string_map_entry_t;

typedef struct
{
  int capacity;
  int mask;
  int length;
  string_map_entry_t *entries;
} string_map_t;

void string_map_init(string_map_t *map, int min_capacity);
void string_map_free(string_map_t *map);
string_map_entry_t *string_map_get_entry(string_map_t *map, string_t *key);
void string_map_inplace_put(string_map_t *map, string_t *key, value_t value);

#endif // STRING_MAP_H
