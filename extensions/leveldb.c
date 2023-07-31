//
// The Hook Programming Language
// leveldb.c
//

#include "leveldb.h"
#include <stdlib.h>
#include <leveldb/c.h>
#include <hook/memory.h>
#include <hook/check.h>
#include <hook/status.h>

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
static int new_options_call(HkState *state, HkValue *args);
static int new_read_options_call(HkState *state, HkValue *args);
static int new_write_options_call(HkState *state, HkValue *args);
static int open_call(HkState *state, HkValue *args);
static int close_call(HkState *state, HkValue *args);
static int put_call(HkState *state, HkValue *args);
static int get_call(HkState *state, HkValue *args);
static int delete_call(HkState *state, HkValue *args);

static inline LeveldbWrapper *leveldb_wrapper_new(leveldb_t *db)
{
  LeveldbWrapper *wrapper = (LeveldbWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &leveldb_wrapper_deinit);
  wrapper->db = db;
  return wrapper;
}

static inline LeveldbOptionsWrapper *leveldb_options_wrapper_new(leveldb_options_t *options)
{
  LeveldbOptionsWrapper *wrapper = (LeveldbOptionsWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &leveldb_options_wrapper_deinit);
  wrapper->options = options;
  return wrapper;
}

static inline LeveldbReadOptionsWrapper *leveldb_read_options_wrapper_new(leveldb_readoptions_t *options)
{
  LeveldbReadOptionsWrapper *wrapper = (LeveldbReadOptionsWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &leveldb_read_options_wrapper_deinit);
  wrapper->options = options;
  return wrapper;
}

static inline LeveldbWriteOptionsWrapper *leveldb_write_options_wrapper_new(leveldb_writeoptions_t *options)
{
  LeveldbWriteOptionsWrapper *wrapper = (LeveldbWriteOptionsWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &leveldb_write_options_wrapper_deinit);
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

static int new_options_call(HkState *state, HkValue *args)
{
  (void) args;
  leveldb_options_t *options = leveldb_options_create();
  LeveldbOptionsWrapper *wrapper = leveldb_options_wrapper_new(options);
  return hk_state_push_userdata(state, (HkUserdata *) wrapper);
}

static int new_read_options_call(HkState *state, HkValue *args)
{
  (void) args;
  leveldb_readoptions_t *options = leveldb_readoptions_create();
  LeveldbReadOptionsWrapper *wrapper = leveldb_read_options_wrapper_new(options);
  return hk_state_push_userdata(state, (HkUserdata *) wrapper);
}

static int new_write_options_call(HkState *state, HkValue *args)
{
  (void) args;
  leveldb_writeoptions_t *options = leveldb_writeoptions_create();
  LeveldbWriteOptionsWrapper *wrapper = leveldb_write_options_wrapper_new(options);
  return hk_state_push_userdata(state, (HkUserdata *) wrapper);
}

static int options_set_create_if_missing_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_bool(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  leveldb_options_t *options = ((LeveldbOptionsWrapper *) hk_as_userdata(args[1]))->options;
  bool on = hk_as_bool(args[2]);
  leveldb_options_set_create_if_missing(options, (uint8_t) on);
  return hk_state_push_nil(state);
}

static int open_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkType types[] = {HK_TYPE_NIL, HK_TYPE_USERDATA};
  if (hk_check_argument_types(args, 2, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
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
    return hk_state_push_array(state, arr);
  }
  LeveldbWrapper *wrapper = leveldb_wrapper_new(db);
  hk_array_inplace_add_element(arr, hk_userdata_value((HkUserdata *) wrapper));
  hk_array_inplace_add_element(arr, HK_NIL_VALUE);
  return hk_state_push_array(state, arr);
}

static int close_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  LeveldbWrapper *wrapper = (LeveldbWrapper *) hk_as_userdata(args[1]);
  leveldb_t *db = wrapper->db;
  if (!db)
  {
    leveldb_close(db);
    wrapper->db = NULL;
  }
  return hk_state_push_nil(state);
}

static int put_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkType types[] = {HK_TYPE_NIL, HK_TYPE_USERDATA};
  if (hk_check_argument_types(args, 2, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 4) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
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
    return hk_state_push_array(state, arr);
  }
  hk_array_inplace_add_element(arr, HK_TRUE_VALUE);
  hk_array_inplace_add_element(arr, HK_NIL_VALUE);
  return hk_state_push_array(state, arr);
}

static int get_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkType types[] = {HK_TYPE_NIL, HK_TYPE_USERDATA};
  if (hk_check_argument_types(args, 2, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
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
    return hk_state_push_array(state, arr);
  }
  hk_array_inplace_add_element(arr, hk_string_value(hk_string_from_chars(value_length, value)));
  hk_array_inplace_add_element(arr, HK_NIL_VALUE);
  free(value);
  return hk_state_push_array(state, arr);
}

static int delete_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkType types[] = {HK_TYPE_NIL, HK_TYPE_USERDATA};
  if (hk_check_argument_types(args, 2, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
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
    return hk_state_push_array(state, arr);
  }
  hk_array_inplace_add_element(arr, HK_TRUE_VALUE);
  hk_array_inplace_add_element(arr, HK_NIL_VALUE);
  return hk_state_push_array(state, arr);
}

HK_LOAD_FN(leveldb)
{
  if (hk_state_push_string_from_chars(state, -1, "leveldb") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "new_options") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "new_options", 0, &new_options_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "new_read_options") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "new_read_options", 0, &new_read_options_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "new_write_options") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "new_write_options", 0, &new_write_options_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "options_set_create_if_missing") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "options_set_create_if_missing", 2, &options_set_create_if_missing_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "open") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "open", 2, &open_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "close") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "close", 1, &close_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "put") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "put", 4, &put_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "get") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "get", 3, &get_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "delete") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "delete", 3, &delete_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 9);
}
