//
// The Hook Programming Language
// string_map.c
//

#include "string_map.h"
#include <stdlib.h>
#include <hook/memory.h>
#include <hook/utils.h>

static inline StringMapEntry *allocate_entries(int capacity);
static inline void grow(StringMap *map);

static inline StringMapEntry *allocate_entries(int capacity)
{
  StringMapEntry *entries = (StringMapEntry *) hk_allocate(sizeof(*entries) * capacity);
  for (int i = 0; i < capacity; ++i)
    entries[i].key = NULL;
  return entries;
}

static inline void grow(StringMap *map)
{
  int length = map->length;
  if (length / STRING_MAP_MAX_LOAD_FACTOR <= map->capacity)
    return;
  int capacity = map->capacity << 1;
  int mask = capacity - 1;
  StringMapEntry *entries = allocate_entries(capacity);
  for (int i = 0, j = 0; i < map->capacity && j < length; ++i)
  {
    StringMapEntry *entry = &map->entries[i];
    HkString *key = entry->key;
    if (!key)
      continue;
    entries[key->hash & mask] = map->entries[i];
    ++j;
  }
  free(map->entries);
  map->entries = entries;
  map->capacity = capacity;
  map->mask = mask;
}

void string_map_init(StringMap *map, int minCapacity)
{
  int capacity = minCapacity < STRING_MAP_MIN_CAPACITY ? STRING_MAP_MIN_CAPACITY : minCapacity;
  capacity = hk_power_of_two_ceil(capacity);
  map->capacity = capacity;
  map->mask = capacity - 1;
  map->length = 0;
  map->entries = allocate_entries(capacity);
}

void string_map_free(StringMap *map)
{
  StringMapEntry *entries = map->entries;
  for (int i = 0, j = 0; i < map->capacity && j < map->length; ++i)
  {
    StringMapEntry *entry = &entries[i];
    HkString *key = entry->key;
    if (!key)
      continue;
    hk_string_release(key);
    hk_value_release(entry->value);
    ++j;
  }
  free(map->entries);
}

StringMapEntry *string_map_get_entry(StringMap *map, HkString *key)
{
  int mask = map->mask;
  StringMapEntry *entries = map->entries;
  int index = hk_string_hash(key) & mask;
  for (;;)
  {
    StringMapEntry *entry = &entries[index];
    if (!entry->key)
      break;
    if (hk_string_equal(key, entry->key))
      return entry;
    index = (index + 1) & mask;
  }
  return NULL;
}

void string_map_inplace_put(StringMap *map, HkString *key, HkValue value)
{
  int mask = map->mask;
  StringMapEntry *entries = map->entries;
  int index = hk_string_hash(key) & mask;
  for (;;)
  {
    StringMapEntry *entry = &entries[index];
    if (!entry->key)
    {
      hk_incr_ref(key);
      hk_value_incr_ref(value);
      entry->key = key;
      entry->value = value;
      ++map->length;
      grow(map);
      return;
    }
    if (hk_string_equal(key, entry->key))
    {
      hk_value_incr_ref(value);
      hk_value_decr_ref(entry->value);
      entry->value = value;
      return;
    }
    index = (index + 1) & mask;
  }
}
