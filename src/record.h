//
// record.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef RECORD_H
#define RECORD_H

#include <hook/string.h>

#define RECORD_MIN_CAPACITY    (1 << 3)
#define RECORD_MAX_LOAD_FACTOR 0.75

typedef struct
{
  HkString *key;
  HkValue  value;
} RecordEntry;

typedef struct
{
  int         capacity;
  int         mask;
  int         length;
  RecordEntry *entries;
} Record;

void record_init(Record *rec, int minCapacity);
void record_deinit(Record *rec);
RecordEntry *record_get_entry(Record *rec, HkString *key);
void record_inplace_put(Record *rec, HkString *key, HkValue value);

#endif // RECORD_H
