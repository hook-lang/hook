//
// record.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "record.h"
#include <stdlib.h>
#include <hook/memory.h>
#include <hook/utils.h>

static inline RecordEntry *allocate_entries(int capacity);
static inline void grow(Record *rec);

static inline RecordEntry *allocate_entries(int capacity)
{
  RecordEntry *entries = (RecordEntry *) hk_allocate(sizeof(*entries) * capacity);
  for (int i = 0; i < capacity; ++i)
    entries[i].key = NULL;
  return entries;
}

static inline void grow(Record *rec)
{
  int length = rec->length;
  if (length / RECORD_MAX_LOAD_FACTOR <= rec->capacity)
    return;
  int capacity = rec->capacity << 1;
  int mask = capacity - 1;
  RecordEntry *entries = allocate_entries(capacity);
  for (int i = 0, j = 0; i < rec->capacity && j < length; ++i)
  {
    RecordEntry *entry = &rec->entries[i];
    HkString *key = entry->key;
    if (!key)
      continue;
    entries[key->hash & mask] = rec->entries[i];
    ++j;
  }
  free(rec->entries);
  rec->entries = entries;
  rec->capacity = capacity;
  rec->mask = mask;
}

void record_init(Record *rec, int minCapacity)
{
  int capacity = minCapacity < RECORD_MIN_CAPACITY ? RECORD_MIN_CAPACITY : minCapacity;
  capacity = hk_power_of_two_ceil(capacity);
  rec->capacity = capacity;
  rec->mask = capacity - 1;
  rec->length = 0;
  rec->entries = allocate_entries(capacity);
}

void record_deinit(Record *rec)
{
  RecordEntry *entries = rec->entries;
  for (int i = 0, j = 0; i < rec->capacity && j < rec->length; ++i)
  {
    RecordEntry *entry = &entries[i];
    HkString *key = entry->key;
    if (!key)
      continue;
    hk_string_release(key);
    hk_value_release(entry->value);
    ++j;
  }
  free(rec->entries);
}

RecordEntry *record_get_entry(Record *rec, HkString *key)
{
  int mask = rec->mask;
  RecordEntry *entries = rec->entries;
  int index = hk_string_hash(key) & mask;
  for (;;)
  {
    RecordEntry *entry = &entries[index];
    if (!entry->key)
      break;
    if (hk_string_equal(key, entry->key))
      return entry;
    index = (index + 1) & mask;
  }
  return NULL;
}

void record_inplace_put(Record *rec, HkString *key, HkValue value)
{
  int mask = rec->mask;
  RecordEntry *entries = rec->entries;
  int index = hk_string_hash(key) & mask;
  for (;;)
  {
    RecordEntry *entry = &entries[index];
    if (!entry->key)
    {
      hk_incr_ref(key);
      hk_value_incr_ref(value);
      entry->key = key;
      entry->value = value;
      ++rec->length;
      grow(rec);
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
