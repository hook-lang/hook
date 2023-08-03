//
// The Hook Programming Language
// sqlite.c
//

#include "sqlite.h"
#include <hook/memory.h>
#include <hook/error.h>
#include "deps/sqlite3.h"

typedef struct
{
  HK_USERDATA_HEADER
  sqlite3 *sqlite;
} SQLiteWrapper;

typedef struct
{
  HK_USERDATA_HEADER
  sqlite3_stmt *sqlite_stmt;
} SQLiteStmtWrapper;

static inline SQLiteWrapper *sqlite_wrapper_new(sqlite3 *sqlite);
static inline SQLiteStmtWrapper *sqlite_stmt_wrapper_new(sqlite3_stmt *sqlite_stmt);
static void sqlite_wrapper_deinit(HkUserdata *udata);
static void sqlite_stmt_wrapper_deinit(HkUserdata *udata);
static void open_call(HkState *state, HkValue *args);
static void close_call(HkState *state, HkValue *args);
static void execute_call(HkState *state, HkValue *args);
static void prepare_call(HkState *state, HkValue *args);
static void finalize_call(HkState *state, HkValue *args);
static void bind_call(HkState *state, HkValue *args);
static void fetch_row_call(HkState *state, HkValue *args);

static inline SQLiteWrapper *sqlite_wrapper_new(sqlite3 *sqlite)
{
  SQLiteWrapper *wrapper = (SQLiteWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &sqlite_wrapper_deinit);
  wrapper->sqlite = sqlite;
  return wrapper;
}

static inline SQLiteStmtWrapper *sqlite_stmt_wrapper_new(sqlite3_stmt *sqlite_stmt)
{
  SQLiteStmtWrapper *wrapper = (SQLiteStmtWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &sqlite_stmt_wrapper_deinit);
  wrapper->sqlite_stmt = sqlite_stmt;
  return wrapper;
}

static void sqlite_wrapper_deinit(HkUserdata *udata)
{
  sqlite3_close(((SQLiteWrapper *) udata)->sqlite);
}

static void sqlite_stmt_wrapper_deinit(HkUserdata *udata)
{
  sqlite3_finalize(((SQLiteStmtWrapper *) udata)->sqlite_stmt);
}

static void open_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkString *filename = hk_as_string(args[1]);
  sqlite3 *sqlite;
  if (sqlite3_open(filename->chars, &sqlite) != SQLITE_OK)
  {
    hk_state_error(state, "cannot open database `%.*s`", filename->length,
      filename->chars);
    sqlite3_close(sqlite);
    return;
  }
  hk_state_push_userdata(state, (HkUserdata *) sqlite_wrapper_new(sqlite));
}

static void close_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_push_number(state, sqlite3_close(((SQLiteWrapper *) hk_as_userdata(args[1]))->sqlite));
}

static void execute_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  sqlite3 *sqlite = ((SQLiteWrapper *) hk_as_userdata(args[1]))->sqlite;
  HkString *sql = hk_as_string(args[2]);
  char *err = NULL;
  if (sqlite3_exec(sqlite, sql->chars, NULL, NULL, &err) != SQLITE_OK)
  {
    hk_state_error(state, "cannot execute SQL: %s", err);
    sqlite3_free(err);
    return;
  }
  hk_state_push_nil(state);
}

static void prepare_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  sqlite3 *sqlite = ((SQLiteWrapper *) hk_as_userdata(args[1]))->sqlite;
  HkString *sql = hk_as_string(args[2]);
  sqlite3_stmt *sqlite_stmt;
  if (sqlite3_prepare_v2(sqlite, sql->chars, sql->length, &sqlite_stmt, NULL) != SQLITE_OK)
  {
    hk_state_error(state, "cannot prepare SQL: %s", sqlite3_errmsg(sqlite));
    return;
  }
  hk_state_push_userdata(state, (HkUserdata *) sqlite_stmt_wrapper_new(sqlite_stmt));
}

static void finalize_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  sqlite3_stmt *sqlite_stmt = ((SQLiteStmtWrapper *) hk_as_userdata(args[1]))->sqlite_stmt;
  hk_state_push_number(state, sqlite3_finalize(sqlite_stmt));
}

static void bind_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  HkType types[] = {HK_TYPE_NIL, HK_TYPE_BOOL, HK_TYPE_NUMBER, HK_TYPE_STRING};
  hk_state_check_argument_types(state, args, 3, 4, types);
  hk_return_if_not_ok(state);
  sqlite3_stmt *sqlite_stmt = ((SQLiteStmtWrapper *) hk_as_userdata(args[1]))->sqlite_stmt;
  int index = (int) hk_as_number(args[2]);
  HkValue val = args[3];
  if (hk_is_nil(val))
  {
    hk_state_push_number(state, sqlite3_bind_null(sqlite_stmt, index));
    return;
  }
  if (hk_is_bool(val))
  {
    hk_state_push_number(state, sqlite3_bind_int(sqlite_stmt, index, (int) hk_as_bool(val)));
    return;
  }
  if (hk_is_number(val))
  {
    double data = hk_as_number(val);
    if (hk_is_int(val))
    {
      hk_state_push_number(state, sqlite3_bind_int64(sqlite_stmt, index, (int64_t) data));
      return;
    }
    hk_state_push_number(state, sqlite3_bind_double(sqlite_stmt, index, data));
    return;
  }
  HkString *str = hk_as_string(val);
  hk_state_push_number(state, sqlite3_bind_text(sqlite_stmt, index, str->chars, str->length, NULL));
}

static void fetch_row_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  sqlite3_stmt *sqlite_stmt = ((SQLiteStmtWrapper *) hk_as_userdata(args[1]))->sqlite_stmt;
  int num_columns = sqlite3_column_count(sqlite_stmt);
  HkArray *row = NULL;
  if (sqlite3_step(sqlite_stmt) == SQLITE_ROW)
  {
    row = hk_array_new_with_capacity(num_columns);
    for (int i = 0; i < num_columns; ++i)
    {
      int type = sqlite3_column_type(sqlite_stmt, i);
      HkValue elem = HK_NIL_VALUE;
      switch (type)
      {
      case SQLITE_NULL:
        break;
      case SQLITE_INTEGER:
        elem = hk_number_value(sqlite3_column_int(sqlite_stmt, i));
        break;
      case SQLITE_FLOAT:
        elem = hk_number_value(sqlite3_column_double(sqlite_stmt, i));
        break;
      case SQLITE_TEXT:
        {
          int length = sqlite3_column_bytes(sqlite_stmt, i);
          char *chars = (char *) sqlite3_column_text(sqlite_stmt, i);
          elem = hk_string_value(hk_string_from_chars(length, chars));
        }
        break;
      case SQLITE_BLOB:
        {
          int length = sqlite3_column_bytes(sqlite_stmt, i);
          char *chars = (char *) sqlite3_column_blob(sqlite_stmt, i);
          elem = hk_string_value(hk_string_from_chars(length, chars));
        }
        break;
      }
      hk_array_inplace_add_element(row, elem);
    }
  }
  if (row)
  {
    hk_state_push_array(state, row);
    return;
  }
  hk_state_push_nil(state);
}

HK_LOAD_MODULE_HANDLER(sqlite)
{
  hk_state_push_string_from_chars(state, -1, "sqlite");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "open");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "open", 1, &open_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "close");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "close", 1, &close_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "execute");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "execute", 2, &execute_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "prepare");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "prepare", 2, &prepare_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "finalize");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "finalize", 1, &finalize_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "bind");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "bind", 3, &bind_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "fetch_row");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "fetch_row", 1, &fetch_row_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 7);
}
