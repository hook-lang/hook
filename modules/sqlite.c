//
// Hook Programming Language
// sqlite.c
//

#include "sqlite.h"
#include <stdlib.h>
#include <sqlite3.h>
#include <assert.h>

typedef struct
{
  HK_USERDATA_HEADER
  sqlite3 *db;
} sqlite_t;

typedef struct
{
  HK_USERDATA_HEADER
  sqlite3_stmt *stmt;
} sqlite_stmt_t;

static inline sqlite_t *sqlite_new(sqlite3 *db);
static inline sqlite_stmt_t *sqlite_stmt_new(sqlite3_stmt *stmt);
static void sqlite_deinit(hk_userdata_t *udata);
static void sqlite_stmt_deinit(hk_userdata_t *udata);
static int open_call(hk_vm_t *vm, hk_value_t *args);
static int close_call(hk_vm_t *vm, hk_value_t *args);
static int execute_call(hk_vm_t *vm, hk_value_t *args);
static int prepare_call(hk_vm_t *vm, hk_value_t *args);
static int finalize_call(hk_vm_t *vm, hk_value_t *args);
static int bind_call(hk_vm_t *vm, hk_value_t *args);
static int fetch_call(hk_vm_t *vm, hk_value_t *args);

static inline sqlite_t *sqlite_new(sqlite3 *db)
{
  sqlite_t *sqlite = (sqlite_t *) hk_allocate(sizeof(*sqlite));
  hk_userdata_init((hk_userdata_t *) sqlite, &sqlite_deinit);
  sqlite->db = db;
  return sqlite;
}

static inline sqlite_stmt_t *sqlite_stmt_new(sqlite3_stmt *stmt)
{
  sqlite_stmt_t *sqlite_stmt = (sqlite_stmt_t *) hk_allocate(sizeof(*sqlite_stmt));
  hk_userdata_init((hk_userdata_t *) sqlite_stmt, &sqlite_stmt_deinit);
  sqlite_stmt->stmt = stmt;
  return sqlite_stmt;
}

static void sqlite_deinit(hk_userdata_t *udata)
{
  sqlite3_close(((sqlite_t *) udata)->db);
}

static void sqlite_stmt_deinit(hk_userdata_t *udata)
{
  sqlite3_finalize(((sqlite_stmt_t *) udata)->stmt);
}

static int open_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *filename = hk_as_string(args[1]);
  sqlite3 *db;
  if (sqlite3_open(filename->chars, &db) != SQLITE_OK)
  {
    hk_runtime_error("cannot open database `%.*s`", filename->length,
      filename->chars);
    sqlite3_close(db);
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_userdata(vm, (hk_userdata_t *) sqlite_new(db));
}

static int close_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_push_number(vm, sqlite3_close(((sqlite_t *) hk_as_userdata(args[1]))->db));
}

static int execute_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  sqlite3 *db = ((sqlite_t *) hk_as_userdata(args[1]))->db;
  hk_string_t *sql = hk_as_string(args[2]);
  char *err = NULL;
  if (sqlite3_exec(db, sql->chars, NULL, NULL, &err) != SQLITE_OK)
  {
    hk_runtime_error("cannot execute SQL: %s", err);
    sqlite3_free(err);
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_nil(vm);
}

static int prepare_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  sqlite3 *db = ((sqlite_t *) hk_as_userdata(args[1]))->db;
  hk_string_t *sql = hk_as_string(args[2]);
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, sql->chars, sql->length, &stmt, NULL) != SQLITE_OK)
  {
    hk_runtime_error("cannot prepare SQL: %s", sqlite3_errmsg(db));
    return HK_STATUS_ERROR;
  }
  return hk_vm_push_userdata(vm, (hk_userdata_t *) sqlite_stmt_new(stmt));
}

static int finalize_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  sqlite3_stmt *stmt = ((sqlite_stmt_t *) hk_as_userdata(args[1]))->stmt;
  return hk_vm_push_number(vm, sqlite3_finalize(stmt));
}

static int bind_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  int types[] = {HK_TYPE_NIL, HK_TYPE_BOOLEAN, HK_TYPE_NUMBER, HK_TYPE_STRING};
  if (hk_vm_check_types(args, 3, 4, types) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  sqlite3_stmt *stmt = ((sqlite_stmt_t *) hk_as_userdata(args[1]))->stmt;
  int index = (int) args[2].as.number;
  hk_value_t val = args[3];
  if (hk_is_nil(val))
    return hk_vm_push_number(vm, sqlite3_bind_null(stmt, index));
  if (hk_is_boolean(val))
    return hk_vm_push_number(vm, sqlite3_bind_int(stmt, index, (int) val.as.boolean));
  if (hk_is_number(val))
  {
    double data = val.as.number;
    if (hk_is_int(val))
      return hk_vm_push_number(vm, sqlite3_bind_int(stmt, index, (int) data));
    return hk_vm_push_number(vm, sqlite3_bind_double(stmt, index, data));
  }
  hk_string_t *str = hk_as_string(val);
  return hk_vm_push_number(vm, sqlite3_bind_text(stmt, index, str->chars, str->length,
    NULL));
}

static int fetch_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  sqlite3_stmt *stmt = ((sqlite_stmt_t *) hk_as_userdata(args[1]))->stmt;
  int num_columns = sqlite3_column_count(stmt);
  hk_array_t *row = NULL;
  if (sqlite3_step(stmt) == SQLITE_ROW)
  {
    row = hk_array_new_with_capacity(num_columns);
    for (int i = 0; i < num_columns; ++i)
    {
      int type = sqlite3_column_type(stmt, i);
      hk_value_t elem = HK_NIL_VALUE;
      switch (type)
      {
      case SQLITE_NULL:
        break;
      case SQLITE_INTEGER:
        elem = hk_number_value(sqlite3_column_int(stmt, i));
        break;
      case SQLITE_FLOAT:
        elem = hk_number_value(sqlite3_column_double(stmt, i));
        break;
      case SQLITE_TEXT:
        {
          int length = sqlite3_column_bytes(stmt, i);
          char *chars = (char *) sqlite3_column_text(stmt, i);
          elem = hk_string_value(hk_string_from_chars(length, chars));
        }
        break;
      case SQLITE_BLOB:
        {
          int length = sqlite3_column_bytes(stmt, i);
          char *chars = (char *) sqlite3_column_blob(stmt, i);
          elem = hk_string_value(hk_string_from_chars(length, chars));
        }
        break;
      }
      hk_array_inplace_add_element(row, elem);
    }
  }
  return row ? hk_vm_push_array(vm, row) : hk_vm_push_nil(vm);
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_sqlite(hk_vm_t *vm)
#else
int load_sqlite(hk_vm_t *vm)
#endif
{
  if (hk_vm_push_string_from_chars(vm, -1, "sqlite") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "open") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "open", 1, &open_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "close") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "close", 1, &close_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "execute") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "execute", 2, &execute_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "prepare") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "prepare", 2, &prepare_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "finalize") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "finalize", 1, &finalize_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "bind") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "bind", 3, &bind_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "fetch") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "fetch", 1, &fetch_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 7);
}
