//
// sqlite.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "sqlite.h"
#include <sqlite3.h>

typedef struct
{
  HK_USERDATA_HEADER
  sqlite3 *sqlite;
} SQLiteWrapper;

typedef struct
{
  HK_USERDATA_HEADER
  sqlite3_stmt *stmt;
} SQLiteStmtWrapper;

static inline SQLiteWrapper *sqlite_wrapper_new(sqlite3 *sqlite);
static inline SQLiteStmtWrapper *sqlite_stmt_wrapper_new(sqlite3_stmt *stmt);
static void sqlite_wrapper_deinit(HkUserdata *udata);
static void sqlite_stmt_wrapper_deinit(HkUserdata *udata);
static void open_call(HkVM *vm, HkValue *args);
static void close_call(HkVM *vm, HkValue *args);
static void execute_call(HkVM *vm, HkValue *args);
static void prepare_call(HkVM *vm, HkValue *args);
static void finalize_call(HkVM *vm, HkValue *args);
static void bind_call(HkVM *vm, HkValue *args);
static void fetch_row_call(HkVM *vm, HkValue *args);

static inline SQLiteWrapper *sqlite_wrapper_new(sqlite3 *sqlite)
{
  SQLiteWrapper *wrapper = (SQLiteWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, sqlite_wrapper_deinit);
  wrapper->sqlite = sqlite;
  return wrapper;
}

static inline SQLiteStmtWrapper *sqlite_stmt_wrapper_new(sqlite3_stmt *stmt)
{
  SQLiteStmtWrapper *wrapper = (SQLiteStmtWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, sqlite_stmt_wrapper_deinit);
  wrapper->stmt = stmt;
  return wrapper;
}

static void sqlite_wrapper_deinit(HkUserdata *udata)
{
  sqlite3_close(((SQLiteWrapper *) udata)->sqlite);
}

static void sqlite_stmt_wrapper_deinit(HkUserdata *udata)
{
  sqlite3_finalize(((SQLiteStmtWrapper *) udata)->stmt);
}

static void open_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *filename = hk_as_string(args[1]);
  sqlite3 *sqlite;
  if (sqlite3_open(filename->chars, &sqlite) != SQLITE_OK)
  {
    hk_vm_runtime_error(vm, "cannot open database `%.*s`", filename->length,
      filename->chars);
    sqlite3_close(sqlite);
    return;
  }
  hk_vm_push_userdata(vm, (HkUserdata *) sqlite_wrapper_new(sqlite));
}

static void close_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_push_number(vm, sqlite3_close(((SQLiteWrapper *) hk_as_userdata(args[1]))->sqlite));
}

static void execute_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  sqlite3 *sqlite = ((SQLiteWrapper *) hk_as_userdata(args[1]))->sqlite;
  HkString *sql = hk_as_string(args[2]);
  char *err = NULL;
  if (sqlite3_exec(sqlite, sql->chars, NULL, NULL, &err) != SQLITE_OK)
  {
    hk_vm_runtime_error(vm, "cannot execute SQL: %s", err);
    sqlite3_free(err);
    return;
  }
  hk_vm_push_nil(vm);
}

static void prepare_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  sqlite3 *sqlite = ((SQLiteWrapper *) hk_as_userdata(args[1]))->sqlite;
  HkString *sql = hk_as_string(args[2]);
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(sqlite, sql->chars, sql->length, &stmt, NULL) != SQLITE_OK)
  {
    hk_vm_runtime_error(vm, "cannot prepare SQL: %s", sqlite3_errmsg(sqlite));
    return;
  }
  hk_vm_push_userdata(vm, (HkUserdata *) sqlite_stmt_wrapper_new(stmt));
}

static void finalize_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  sqlite3_stmt *stmt = ((SQLiteStmtWrapper *) hk_as_userdata(args[1]))->stmt;
  hk_vm_push_number(vm, sqlite3_finalize(stmt));
}

static void bind_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkType types[] = { HK_TYPE_NIL, HK_TYPE_BOOL, HK_TYPE_NUMBER, HK_TYPE_STRING };
  hk_vm_check_argument_types(vm, args, 3, 4, types);
  hk_return_if_not_ok(vm);
  sqlite3_stmt *stmt = ((SQLiteStmtWrapper *) hk_as_userdata(args[1]))->stmt;
  int index = (int) hk_as_number(args[2]);
  HkValue val = args[3];
  if (hk_is_nil(val))
  {
    hk_vm_push_number(vm, sqlite3_bind_null(stmt, index));
    return;
  }
  if (hk_is_bool(val))
  {
    hk_vm_push_number(vm, sqlite3_bind_int(stmt, index, (int) hk_as_bool(val)));
    return;
  }
  if (hk_is_number(val))
  {
    double data = hk_as_number(val);
    if (hk_is_int(val))
    {
      hk_vm_push_number(vm, sqlite3_bind_int64(stmt, index, (int64_t) data));
      return;
    }
    hk_vm_push_number(vm, sqlite3_bind_double(stmt, index, data));
    return;
  }
  HkString *str = hk_as_string(val);
  hk_vm_push_number(vm, sqlite3_bind_text(stmt, index, str->chars, str->length, NULL));
}

static void fetch_row_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  sqlite3_stmt *stmt = ((SQLiteStmtWrapper *) hk_as_userdata(args[1]))->stmt;
  int num_columns = sqlite3_column_count(stmt);
  HkArray *row = NULL;
  if (sqlite3_step(stmt) == SQLITE_ROW)
  {
    row = hk_array_new_with_capacity(num_columns);
    for (int i = 0; i < num_columns; ++i)
    {
      int type = sqlite3_column_type(stmt, i);
      HkValue elem = HK_NIL_VALUE;
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
  if (row)
  {
    hk_vm_push_array(vm, row);
    return;
  }
  hk_vm_push_nil(vm);
}

HK_LOAD_MODULE_HANDLER(sqlite)
{
  hk_vm_push_string_from_chars(vm, -1, "sqlite");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "open");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "open", 1, open_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "close");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "close", 1, close_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "execute");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "execute", 2, execute_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "prepare");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "prepare", 2, prepare_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "finalize");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "finalize", 1, finalize_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "bind");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "bind", 3, bind_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "fetch_row");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "fetch_row", 1, fetch_row_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 7);
}
