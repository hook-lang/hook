//
// leveldb.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "leveldb.h"
#include <stdlib.h>
#include <leveldb/c.h>

typedef struct
{
  HK_USERDATA_HEADER
  leveldb_t *db;
} LeveldbWrapper;

typedef struct
{
  HK_USERDATA_HEADER
  leveldb_options_t *options;
} LeveldbOptionsWrapper;

typedef struct
{
  HK_USERDATA_HEADER
  leveldb_readoptions_t *options;
} LeveldbReadOptionsWrapper;

typedef struct
{
  HK_USERDATA_HEADER
  leveldb_writeoptions_t *options;
} LeveldbWriteOptionsWrapper;

static inline LeveldbWrapper *leveldb_wrapper_new(leveldb_t *db);
static inline LeveldbOptionsWrapper *leveldb_options_wrapper_new(leveldb_options_t *options);
static inline LeveldbReadOptionsWrapper *leveldb_read_options_wrapper_new(leveldb_readoptions_t *options);
static inline LeveldbWriteOptionsWrapper *leveldb_write_options_wrapper_new(leveldb_writeoptions_t *options);
static void leveldb_wrapper_deinit(HkUserdata *udata);
static void leveldb_options_wrapper_deinit(HkUserdata *udata);
static void leveldb_read_options_wrapper_deinit(HkUserdata *udata);
static void leveldb_write_options_wrapper_deinit(HkUserdata *udata);
static void new_options_call(HkVM *vm, HkValue *args);
static void new_read_options_call(HkVM *vm, HkValue *args);
static void new_write_options_call(HkVM *vm, HkValue *args);
static void open_call(HkVM *vm, HkValue *args);
static void close_call(HkVM *vm, HkValue *args);
static void put_call(HkVM *vm, HkValue *args);
static void get_call(HkVM *vm, HkValue *args);
static void delete_call(HkVM *vm, HkValue *args);

static inline LeveldbWrapper *leveldb_wrapper_new(leveldb_t *db)
{
  LeveldbWrapper *wrapper = (LeveldbWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, leveldb_wrapper_deinit);
  wrapper->db = db;
  return wrapper;
}

static inline LeveldbOptionsWrapper *leveldb_options_wrapper_new(leveldb_options_t *options)
{
  LeveldbOptionsWrapper *wrapper = (LeveldbOptionsWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, leveldb_options_wrapper_deinit);
  wrapper->options = options;
  return wrapper;
}

static inline LeveldbReadOptionsWrapper *leveldb_read_options_wrapper_new(leveldb_readoptions_t *options)
{
  LeveldbReadOptionsWrapper *wrapper = (LeveldbReadOptionsWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, leveldb_read_options_wrapper_deinit);
  wrapper->options = options;
  return wrapper;
}

static inline LeveldbWriteOptionsWrapper *leveldb_write_options_wrapper_new(leveldb_writeoptions_t *options)
{
  LeveldbWriteOptionsWrapper *wrapper = (LeveldbWriteOptionsWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, leveldb_write_options_wrapper_deinit);
  wrapper->options = options;
  return wrapper;
}

static void leveldb_wrapper_deinit(HkUserdata *udata)
{
  LeveldbWrapper *wrapper = (LeveldbWrapper *) udata;
  if (!wrapper->db)
    return;
  leveldb_close(wrapper->db);
}

static void leveldb_options_wrapper_deinit(HkUserdata *udata)
{
  leveldb_options_destroy(((LeveldbOptionsWrapper *) udata)->options);
}

static void leveldb_read_options_wrapper_deinit(HkUserdata *udata)
{
  leveldb_readoptions_destroy(((LeveldbReadOptionsWrapper *) udata)->options);
}

static void leveldb_write_options_wrapper_deinit(HkUserdata *udata)
{
  leveldb_writeoptions_destroy(((LeveldbWriteOptionsWrapper *) udata)->options);
}

static void new_options_call(HkVM *vm, HkValue *args)
{
  (void) args;
  leveldb_options_t *options = leveldb_options_create();
  LeveldbOptionsWrapper *wrapper = leveldb_options_wrapper_new(options);
  hk_vm_push_userdata(vm, (HkUserdata *) wrapper);
}

static void new_read_options_call(HkVM *vm, HkValue *args)
{
  (void) args;
  leveldb_readoptions_t *options = leveldb_readoptions_create();
  LeveldbReadOptionsWrapper *wrapper = leveldb_read_options_wrapper_new(options);
  hk_vm_push_userdata(vm, (HkUserdata *) wrapper);
}

static void new_write_options_call(HkVM *vm, HkValue *args)
{
  (void) args;
  leveldb_writeoptions_t *options = leveldb_writeoptions_create();
  LeveldbWriteOptionsWrapper *wrapper = leveldb_write_options_wrapper_new(options);
  hk_vm_push_userdata(vm, (HkUserdata *) wrapper);
}

static void options_set_create_if_missing_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_bool(vm, args, 2);
  hk_return_if_not_ok(vm);
  leveldb_options_t *options = ((LeveldbOptionsWrapper *) hk_as_userdata(args[1]))->options;
  bool on = hk_as_bool(args[2]);
  leveldb_options_set_create_if_missing(options, (uint8_t) on);
  hk_vm_push_nil(vm);
}

static void open_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkType types[] = { HK_TYPE_NIL, HK_TYPE_USERDATA };
  hk_vm_check_argument_types(vm, args, 2, 2, types);
  hk_return_if_not_ok(vm);
  HkString *name = hk_as_string(args[1]);
  HkValue val = args[2];
  leveldb_options_t *options = hk_is_nil(val) ?  leveldb_options_create() :
    ((LeveldbOptionsWrapper *) hk_as_userdata(val))->options;
  char *err = NULL;
  leveldb_t *db = leveldb_open(options, name->chars, &err);
  HkArray *arr = hk_array_new_with_capacity(2);
  if (err)
  {
    hk_array_inplace_add_element(arr, HK_NIL_VALUE);
    hk_array_inplace_add_element(arr, hk_string_value(hk_string_from_chars(-1, err)));
    free(err);
    hk_vm_push_array(vm, arr);
    return;
  }
  LeveldbWrapper *wrapper = leveldb_wrapper_new(db);
  hk_array_inplace_add_element(arr, hk_userdata_value((HkUserdata *) wrapper));
  hk_array_inplace_add_element(arr, HK_NIL_VALUE);
  hk_vm_push_array(vm, arr);
}

static void close_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  LeveldbWrapper *wrapper = (LeveldbWrapper *) hk_as_userdata(args[1]);
  leveldb_t *db = wrapper->db;
  if (!db)
  {
    leveldb_close(db);
    wrapper->db = NULL;
  }
  hk_vm_push_nil(vm);
}

static void put_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkType types[] = { HK_TYPE_NIL, HK_TYPE_USERDATA };
  hk_vm_check_argument_types(vm, args, 2, 2, types);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 3);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 4);
  hk_return_if_not_ok(vm);
  leveldb_t *db = ((LeveldbWrapper *) hk_as_userdata(args[1]))->db;
  HkValue val = args[2];
  leveldb_writeoptions_t *options = hk_is_nil(val) ?  leveldb_writeoptions_create() :
    ((LeveldbWriteOptionsWrapper *) hk_as_userdata(val))->options;
  HkString *key = hk_as_string(args[3]);
  HkString *value = hk_as_string(args[4]);
  char *err = NULL;
  leveldb_put(db, options, key->chars, key->length, value->chars, value->length, &err);
  HkArray *arr = hk_array_new_with_capacity(2);
  if (err)
  {
    hk_array_inplace_add_element(arr, HK_FALSE_VALUE);
    hk_array_inplace_add_element(arr, hk_string_value(hk_string_from_chars(-1, err)));
    free(err);
    hk_vm_push_array(vm, arr);
    return;
  }
  hk_array_inplace_add_element(arr, HK_TRUE_VALUE);
  hk_array_inplace_add_element(arr, HK_NIL_VALUE);
  hk_vm_push_array(vm, arr);
}

static void get_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkType types[] = { HK_TYPE_NIL, HK_TYPE_USERDATA };
  hk_vm_check_argument_types(vm, args, 2, 2, types);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 3);
  hk_return_if_not_ok(vm);
  leveldb_t *db = ((LeveldbWrapper *) hk_as_userdata(args[1]))->db;
  HkValue val = args[2];
  leveldb_readoptions_t *options = hk_is_nil(val) ?  leveldb_readoptions_create() :
    ((LeveldbReadOptionsWrapper *) hk_as_userdata(val))->options;
  HkString *key = hk_as_string(args[3]);
  size_t value_length;
  char *err = NULL;
  char *value = leveldb_get(db, options, key->chars, key->length, &value_length, &err);
  HkArray *arr = hk_array_new_with_capacity(2);
  if (err)
  {
    hk_array_inplace_add_element(arr, HK_NIL_VALUE);
    hk_array_inplace_add_element(arr, hk_string_value(hk_string_from_chars(-1, err)));
    free(err);
    hk_vm_push_array(vm, arr);
    return;
  }
  hk_array_inplace_add_element(arr, hk_string_value(hk_string_from_chars(value_length, value)));
  hk_array_inplace_add_element(arr, HK_NIL_VALUE);
  free(value);
  hk_vm_push_array(vm, arr);
}

static void delete_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkType types[] = { HK_TYPE_NIL, HK_TYPE_USERDATA };
  hk_vm_check_argument_types(vm, args, 2, 2, types);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 3);
  hk_return_if_not_ok(vm);
  leveldb_t *db = ((LeveldbWrapper *) hk_as_userdata(args[1]))->db;
  HkValue val = args[2];
  leveldb_writeoptions_t *options = hk_is_nil(val) ?  leveldb_writeoptions_create() :
    ((LeveldbWriteOptionsWrapper *) hk_as_userdata(val))->options;
  HkString *key = hk_as_string(args[3]);
  char *err = NULL;
  leveldb_delete(db, options, key->chars, key->length, &err);
  HkArray *arr = hk_array_new_with_capacity(2);
  if (err)
  {
    hk_array_inplace_add_element(arr, HK_FALSE_VALUE);
    hk_array_inplace_add_element(arr, hk_string_value(hk_string_from_chars(-1, err)));
    free(err);
    hk_vm_push_array(vm, arr);
    return;
  }
  hk_array_inplace_add_element(arr, HK_TRUE_VALUE);
  hk_array_inplace_add_element(arr, HK_NIL_VALUE);
  hk_vm_push_array(vm, arr);
}

HK_LOAD_MODULE_HANDLER(leveldb)
{
  hk_vm_push_string_from_chars(vm, -1, "leveldb");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "new_options");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "new_options", 0, new_options_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "new_read_options");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "new_read_options", 0, new_read_options_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "new_write_options");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "new_write_options", 0, new_write_options_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "options_set_create_if_missing");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "options_set_create_if_missing", 2, options_set_create_if_missing_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "open");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "open", 2, open_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "close");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "close", 1, close_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "put");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "put", 4, put_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "get");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "get", 3, get_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "delete");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "delete", 3, delete_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 9);
}
