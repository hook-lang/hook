//
// Hook Programming Language
// sqlite.c
//

#include "sqlite.h"
#include <stdlib.h>
#include <sqlite3.h>
#include <assert.h>
#include "common.h"
#include "memory.h"
#include "error.h"

typedef struct
{
  USERDATA_HEADER
  sqlite3 *db;
} sqlite_t;

typedef struct
{
  USERDATA_HEADER
  sqlite3_stmt *stmt;
} sqlite_stmt_t;

static inline sqlite_t *sqlite_new(sqlite3 *db);
static inline sqlite_stmt_t *sqlite_stmt_new(sqlite3_stmt *stmt);
static void sqlite_deinit(userdata_t *udata);
static void sqlite_stmt_deinit(userdata_t *udata);
static int open_call(vm_t *vm, value_t *args);
static int close_call(vm_t *vm, value_t *args);
static int execute_call(vm_t *vm, value_t *args);
static int prepare_call(vm_t *vm, value_t *args);
static int finalize_call(vm_t *vm, value_t *args);
static int bind_call(vm_t *vm, value_t *args);
static int fetch_call(vm_t *vm, value_t *args);

static inline sqlite_t *sqlite_new(sqlite3 *db)
{
  sqlite_t *sqlite = (sqlite_t *) allocate(sizeof(*sqlite));
  userdata_init((userdata_t *) sqlite, &sqlite_deinit);
  sqlite->db = db;
  return sqlite;
}

static inline sqlite_stmt_t *sqlite_stmt_new(sqlite3_stmt *stmt)
{
  sqlite_stmt_t *sqlite_stmt = (sqlite_stmt_t *) allocate(sizeof(*sqlite_stmt));
  userdata_init((userdata_t *) sqlite_stmt, &sqlite_stmt_deinit);
  sqlite_stmt->stmt = stmt;
  return sqlite_stmt;
}

static void sqlite_deinit(userdata_t *udata)
{
  sqlite3_close(((sqlite_t *) udata)->db);
}

static void sqlite_stmt_deinit(userdata_t *udata)
{
  sqlite3_finalize(((sqlite_stmt_t *) udata)->stmt);
}

static int open_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_STRING(val))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  string_t *filename = AS_STRING(val);
  sqlite3 *db;
  if (sqlite3_open(filename->chars, &db) != SQLITE_OK) {
    runtime_error("invalid type: cannot open database `%.*s`", filename->length, filename->chars);
    sqlite3_close(db);
    return STATUS_ERROR;
  }
  return vm_push_userdata(vm, (userdata_t *) sqlite_new(db));
}

static int close_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  return vm_push_number(vm, sqlite3_close(((sqlite_t *) AS_USERDATA(val))->db));
}

static int execute_call(vm_t *vm, value_t *args)
{
  value_t val1 = args[1];
  value_t val2 = args[2];
  if (!IS_USERDATA(val1))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  sqlite3 *db = ((sqlite_t *) AS_USERDATA(val1))->db;
  string_t *sql = AS_STRING(val2);
  char *err = NULL;
  if (sqlite3_exec(db, sql->chars, NULL, NULL, &err) != SQLITE_OK)
  {
    runtime_error("cannot execute SQL: %s", err);
    sqlite3_free(err);
    return STATUS_ERROR;
  }
  return vm_push_null(vm);
}

static int prepare_call(vm_t *vm, value_t *args)
{
  value_t val1 = args[1];
  value_t val2 = args[2];
  if (!IS_USERDATA(val1))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_STRING(val2))
  {
    runtime_error("invalid type: expected string but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  sqlite3 *db = ((sqlite_t *) AS_USERDATA(val1))->db;
  string_t *sql = AS_STRING(val2);
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, sql->chars, sql->length, &stmt, NULL) != SQLITE_OK)
  {
    runtime_error("cannot prepare SQL: %s", sqlite3_errmsg(db));
    return STATUS_ERROR;
  }
  return vm_push_userdata(vm, (userdata_t *) sqlite_stmt_new(stmt));
}

static int finalize_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  sqlite3_stmt *stmt = ((sqlite_stmt_t *) AS_USERDATA(val))->stmt;
  return vm_push_number(vm, sqlite3_finalize(stmt));
}

static int bind_call(vm_t *vm, value_t *args)
{
  value_t val1 = args[1];
  value_t val2 = args[2];
  value_t val3 = args[3];
  if (!IS_USERDATA(val1))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val1.type));
    return STATUS_ERROR;
  }
  if (!IS_INTEGER(val2))
  {
    runtime_error("invalid type: expected integer but got `%s`", type_name(val2.type));
    return STATUS_ERROR;
  }
  sqlite3_stmt *stmt = ((sqlite_stmt_t *) AS_USERDATA(val1))->stmt;
  int index = (int) val2.as.number;
  int rc = SQLITE_OK;
  switch (val3.type)
  {
  case TYPE_NULL:
    rc = sqlite3_bind_null(stmt, index);
    break;
  case TYPE_BOOLEAN:
    rc = sqlite3_bind_int(stmt, index, (int) val3.as.boolean);
    break;
  case TYPE_NUMBER:
    {
      if (IS_INTEGER(val3))
      {
        rc = sqlite3_bind_int(stmt, index, (int) val3.as.number);
        break;
      }
      rc = sqlite3_bind_double(stmt, index, val3.as.number);
    }
    break;
  case TYPE_STRING:
    {
      string_t *str = AS_STRING(val3);
      rc = sqlite3_bind_text(stmt, index, str->chars, str->length, NULL);
    }
    break;
  case TYPE_ARRAY:
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    {
      runtime_error("cannot bind value of type `%s`", type_name(val3.type));
      return STATUS_ERROR;
    }
    break;
  }
  return vm_push_number(vm, rc);
}

static int fetch_call(vm_t *vm, value_t *args)
{
  value_t val = args[1];
  if (!IS_USERDATA(val))
  {
    runtime_error("invalid type: expected userdata but got `%s`", type_name(val.type));
    return STATUS_ERROR;
  }
  sqlite3_stmt *stmt = ((sqlite_stmt_t *) AS_USERDATA(val))->stmt;
  int num_columns = sqlite3_column_count(stmt);
  array_t *row = NULL;
  if (sqlite3_step(stmt) == SQLITE_ROW)
  {
    row = array_new(num_columns);
    for (int i = 0; i < num_columns; ++i)
    {
      int type = sqlite3_column_type(stmt, i);
      value_t elem = NULL_VALUE;
      switch (type)
      {
      case SQLITE_NULL:
        break;
      case SQLITE_INTEGER:
        elem = NUMBER_VALUE(sqlite3_column_int(stmt, i));
        break;
      case SQLITE_FLOAT:
        elem = NUMBER_VALUE(sqlite3_column_double(stmt, i));
        break;
      case SQLITE_TEXT:
        {
          int length = sqlite3_column_bytes(stmt, i);
          char *chars = (char *) sqlite3_column_text(stmt, i);
          elem = STRING_VALUE(string_from_chars(length, chars));
        }
        break;
      case SQLITE_BLOB:
        {
          int length = sqlite3_column_bytes(stmt, i);
          char *chars = (char *) sqlite3_column_blob(stmt, i);
          elem = STRING_VALUE(string_from_chars(length, chars));
        }
        break;
      }
      array_inplace_add_element(row, elem);
    }
  }
  return row ? vm_push_array(vm, row) : vm_push_null(vm);
}

#ifdef _WIN32
void __declspec(dllexport) __stdcall load_sqlite(vm_t *vm)
#else
void load_sqlite(vm_t *vm)
#endif
{
  vm_push_string_from_chars(vm, -1, "sqlite");
  vm_push_string_from_chars(vm, -1, "open");
  vm_push_new_native(vm, "open", 1, &open_call);
  vm_push_string_from_chars(vm, -1, "close");
  vm_push_new_native(vm, "close", 1, &close_call);
  vm_push_string_from_chars(vm, -1, "execute");
  vm_push_new_native(vm, "execute", 2, &execute_call);
  vm_push_string_from_chars(vm, -1, "prepare");
  vm_push_new_native(vm, "prepare", 2, &prepare_call);
  vm_push_string_from_chars(vm, -1, "finalize");
  vm_push_new_native(vm, "finalize", 1, &finalize_call);
  vm_push_string_from_chars(vm, -1, "bind");
  vm_push_new_native(vm, "bind", 3, &bind_call);
  vm_push_string_from_chars(vm, -1, "fetch");
  vm_push_new_native(vm, "fetch", 1, &fetch_call);
  ASSERT(vm_construct(vm, 7) == STATUS_OK, "cannot load library `sqlite`");
}
