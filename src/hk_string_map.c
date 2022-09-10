//
// The Hook Programming Language
// hk_string_map.c
//

#include "hk_string_map.h"
#include <stdlib.h>
#include "hk_memory.h"
#include "hk_utils.h"

static inline string_map_entry_t *allocate_entries(int32_t capacity);
static inline void grow(string_map_t *map);

static inline string_map_entry_t *allocate_entries(int32_t capacity)
{
  string_map_entry_t *entries = (string_map_entry_t *) hk_allocate(sizeof(*entries) * capacity);
  for (int32_t i = 0; i < capacity; ++i)
    entries[i].key = NULL;
  return entries;
}

static inline void grow(string_map_t *map)
{
  int32_t length = map->length;
  if (length / STRING_MAP_MAX_LOAD_FACTOR <= map->capacity)
    return;
  int32_t capacity = map->capacity << 1;
  int32_t mask = capacity - 1;
  string_map_entry_t *entries = allocate_entries(capacity);
  for (int32_t i = 0, j = 0; i < map->capacity && j < length; ++i)
  {
    string_map_entry_t *entry = &map->entries[i];
    hk_string_t *key = entry->key;
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

void string_map_init(string_map_t *map, int32_t min_capacity)
{
  int32_t capacity = min_capacity < STRING_MAP_MIN_CAPACITY ? STRING_MAP_MIN_CAPACITY : min_capacity;
  capacity = hk_power_of_two_ceil(capacity);
  map->capacity = capacity;
  map->mask = capacity - 1;
  map->length = 0;
  map->entries = allocate_entries(capacity);
}

void string_map_free(string_map_t *map)
{
  string_map_entry_t *entries = map->entries;
  for (int32_t i = 0, j = 0; i < map->capacity && j < map->length; ++i)
  {
    string_map_entry_t *entry = &entries[i];
    hk_string_t *key = entry->key;
    if (!key)
      continue;
    hk_string_release(key);
    hk_value_release(entry->value);
    ++j;
  }
  free(map->entries);
}

string_map_entry_t *string_map_get_entry(string_map_t *map, hk_string_t *key)
{
  int32_t mask = map->mask;
  string_map_entry_t *entries = map->entries;
  int32_t index = hk_string_hash(key) & mask;
  for (;;)
  {
    string_map_entry_t *entry = &entries[index];
    if (!entry->key)
      break;
    if (hk_string_equal(key, entry->key))
      return entry;
    index = (index + 1) & mask;
  }
  return NULL;
}

void string_map_inplace_put(string_map_t *map, hk_string_t *key, hk_value_t value)
{
  int32_t mask = map->mask;
  string_map_entry_t *entries = map->entries;
  int32_t index = hk_string_hash(key) & mask;
  for (;;)
  {
    string_map_entry_t *entry = &entries[index];
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
