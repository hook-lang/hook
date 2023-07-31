//
// The Hook Programming Language
// string_map.h
//

#ifndef STRING_MAP_H
#define STRING_MAP_H

#include <hook/string.h>

#define STRING_MAP_MIN_CAPACITY    (1 << 3)
#define STRING_MAP_MAX_LOAD_FACTOR 0.75

typedef struct
{
  HkString *key;
  HkValue value;
} StringMapEntry;

typedef struct
{
  int capacity;
  int mask;
  int length;
  StringMapEntry *entries;
} StringMap;

void string_map_init(StringMap *map, int minCapacity);
void string_map_free(StringMap *map);
StringMapEntry *string_map_get_entry(StringMap *map, HkString *key);
void string_map_inplace_put(StringMap *map, HkString *key, HkValue value);

#endif // STRING_MAP_H
