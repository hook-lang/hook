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
} leveldb_wrapper_t;

typedef struct
{
  HK_USERDATA_HEADER
  leveldb_options_t *options;
} leveldb_options_wrapper_t;

typedef struct
{
  HK_USERDATA_HEADER
  leveldb_readoptions_t *options;
} leveldb_read_options_wrapper_t;

typedef struct
{
  HK_USERDATA_HEADER
  leveldb_writeoptions_t *options;
} leveldb_write_options_wrapper_t;

static inline leveldb_wrapper_t *leveldb_wrapper_new(leveldb_t *db);
static inline leveldb_options_wrapper_t *leveldb_options_wrapper_new(leveldb_options_t *options);
static inline leveldb_read_options_wrapper_t *leveldb_read_options_wrapper_new(leveldb_readoptions_t *options);
static inline leveldb_write_options_wrapper_t *leveldb_write_options_wrapper_new(leveldb_writeoptions_t *options);
static void leveldb_wrapper_deinit(hk_userdata_t *udata);
static void leveldb_options_wrapper_deinit(hk_userdata_t *udata);
static void leveldb_read_options_wrapper_deinit(hk_userdata_t *udata);
static void leveldb_write_options_wrapper_deinit(hk_userdata_t *udata);
static int32_t new_options_call(hk_state_t *state, hk_value_t *args);
static int32_t new_read_options_call(hk_state_t *state, hk_value_t *args);
static int32_t new_write_options_call(hk_state_t *state, hk_value_t *args);
static int32_t open_call(hk_state_t *state, hk_value_t *args);
static int32_t close_call(hk_state_t *state, hk_value_t *args);
static int32_t put_call(hk_state_t *state, hk_value_t *args);
static int32_t get_call(hk_state_t *state, hk_value_t *args);
static int32_t delete_call(hk_state_t *state, hk_value_t *args);

static inline leveldb_wrapper_t *leveldb_wrapper_new(leveldb_t *db)
{
  leveldb_wrapper_t *wrapper = (leveldb_wrapper_t *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((hk_userdata_t *) wrapper, &leveldb_wrapper_deinit);
  wrapper->db = db;
  return wrapper;
}

static inline leveldb_options_wrapper_t *leveldb_options_wrapper_new(leveldb_options_t *options)
{
  leveldb_options_wrapper_t *wrapper = (leveldb_options_wrapper_t *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((hk_userdata_t *) wrapper, &leveldb_options_wrapper_deinit);
  wrapper->options = options;
  return wrapper;
}

static inline leveldb_read_options_wrapper_t *leveldb_read_options_wrapper_new(leveldb_readoptions_t *options)
{
  leveldb_read_options_wrapper_t *wrapper = (leveldb_read_options_wrapper_t *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((hk_userdata_t *) wrapper, &leveldb_read_options_wrapper_deinit);
  wrapper->options = options;
  return wrapper;
}

static inline leveldb_write_options_wrapper_t *leveldb_write_options_wrapper_new(leveldb_writeoptions_t *options)
{
  leveldb_write_options_wrapper_t *wrapper = (leveldb_write_options_wrapper_t *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((hk_userdata_t *) wrapper, &leveldb_write_options_wrapper_deinit);
  wrapper->options = options;
  return wrapper;
}

static void leveldb_wrapper_deinit(hk_userdata_t *udata)
{
  leveldb_wrapper_t *wrapper = (leveldb_wrapper_t *) udata;
  if (!wrapper->db)
    return;
  leveldb_close(wrapper->db);
}

static void leveldb_options_wrapper_deinit(hk_userdata_t *udata)
{
  leveldb_options_destroy(((leveldb_options_wrapper_t *) udata)->options);
}

static void leveldb_read_options_wrapper_deinit(hk_userdata_t *udata)
{
  leveldb_readoptions_destroy(((leveldb_read_options_wrapper_t *) udata)->options);
}

static void leveldb_write_options_wrapper_deinit(hk_userdata_t *udata)
{
  leveldb_writeoptions_destroy(((leveldb_write_options_wrapper_t *) udata)->options);
}

static int32_t new_options_call(hk_state_t *state, hk_value_t *args)
{
  (void) args;
  leveldb_options_t *options = leveldb_options_create();
  leveldb_options_wrapper_t *wrapper = leveldb_options_wrapper_new(options);
  return hk_state_push_userdata(state, (hk_userdata_t *) wrapper);
}

static int32_t new_read_options_call(hk_state_t *state, hk_value_t *args)
{
  (void) args;
  leveldb_readoptions_t *options = leveldb_readoptions_create();
  leveldb_read_options_wrapper_t *wrapper = leveldb_read_options_wrapper_new(options);
  return hk_state_push_userdata(state, (hk_userdata_t *) wrapper);
}

static int32_t new_write_options_call(hk_state_t *state, hk_value_t *args)
{
  (void) args;
  leveldb_writeoptions_t *options = leveldb_writeoptions_create();
  leveldb_write_options_wrapper_t *wrapper = leveldb_write_options_wrapper_new(options);
  return hk_state_push_userdata(state, (hk_userdata_t *) wrapper);
}

static int32_t options_set_create_if_missing_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_bool(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  leveldb_options_t *options = ((leveldb_options_wrapper_t *) hk_as_userdata(args[1]))->options;
  bool on = hk_as_bool(args[2]);
  leveldb_options_set_create_if_missing(options, (uint8_t) on);
  return hk_state_push_nil(state);
}

static int32_t open_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_type_t types[] = {HK_TYPE_NIL, HK_TYPE_USERDATA};
  if (hk_check_argument_types(args, 2, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *name = hk_as_string(args[1]);
  hk_value_t val = args[2];
  leveldb_options_t *options = hk_is_nil(val) ?  leveldb_options_create() :
    ((leveldb_options_wrapper_t *) hk_as_userdata(val))->options;
  char *err = NULL;
  leveldb_t *db = leveldb_open(options, name->chars, &err);
  hk_array_t *arr = hk_array_new_with_capacity(2);
  if (err)
  {
    hk_array_inplace_add_element(arr, HK_NIL_VALUE);
    hk_array_inplace_add_element(arr, hk_string_value(hk_string_from_chars(-1, err)));
    free(err);
    return hk_state_push_array(state, arr);
  }
  leveldb_wrapper_t *wrapper = leveldb_wrapper_new(db);
  hk_array_inplace_add_element(arr, hk_userdata_value((hk_userdata_t *) wrapper));
  hk_array_inplace_add_element(arr, HK_NIL_VALUE);
  return hk_state_push_array(state, arr);
}

static int32_t close_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  leveldb_wrapper_t *wrapper = (leveldb_wrapper_t *) hk_as_userdata(args[1]);
  leveldb_t *db = wrapper->db;
  if (!db)
  {
    leveldb_close(db);
    wrapper->db = NULL;
  }
  return hk_state_push_nil(state);
}

static int32_t put_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_type_t types[] = {HK_TYPE_NIL, HK_TYPE_USERDATA};
  if (hk_check_argument_types(args, 2, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 4) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  leveldb_t *db = ((leveldb_wrapper_t *) hk_as_userdata(args[1]))->db;
  hk_value_t val = args[2];
  leveldb_writeoptions_t *options = hk_is_nil(val) ?  leveldb_writeoptions_create() :
    ((leveldb_write_options_wrapper_t *) hk_as_userdata(val))->options;
  hk_string_t *key = hk_as_string(args[3]);
  hk_string_t *value = hk_as_string(args[4]);
  char *err = NULL;
  leveldb_put(db, options, key->chars, key->length, value->chars, value->length, &err);
  hk_array_t *arr = hk_array_new_with_capacity(2);
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

static int32_t get_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_type_t types[] = {HK_TYPE_NIL, HK_TYPE_USERDATA};
  if (hk_check_argument_types(args, 2, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  leveldb_t *db = ((leveldb_wrapper_t *) hk_as_userdata(args[1]))->db;
  hk_value_t val = args[2];
  leveldb_readoptions_t *options = hk_is_nil(val) ?  leveldb_readoptions_create() :
    ((leveldb_read_options_wrapper_t *) hk_as_userdata(val))->options;
  hk_string_t *key = hk_as_string(args[3]);
  size_t value_length;
  char *err = NULL;
  char *value = leveldb_get(db, options, key->chars, key->length, &value_length, &err);
  hk_array_t *arr = hk_array_new_with_capacity(2);
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

static int32_t delete_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_type_t types[] = {HK_TYPE_NIL, HK_TYPE_USERDATA};
  if (hk_check_argument_types(args, 2, 2, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  leveldb_t *db = ((leveldb_wrapper_t *) hk_as_userdata(args[1]))->db;
  hk_value_t val = args[2];
  leveldb_writeoptions_t *options = hk_is_nil(val) ?  leveldb_writeoptions_create() :
    ((leveldb_write_options_wrapper_t *) hk_as_userdata(val))->options;
  hk_string_t *key = hk_as_string(args[3]);
  char *err = NULL;
  leveldb_delete(db, options, key->chars, key->length, &err);
  hk_array_t *arr = hk_array_new_with_capacity(2);
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
