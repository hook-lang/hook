//
// mysql.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "mysql.h"
#include <mysql/mysql.h>
#include <assert.h>

typedef struct
{
  HK_USERDATA_HEADER
  MYSQL *mysql;
} MySQLWrapper;

typedef struct
{
  HK_USERDATA_HEADER
  MYSQL_RES *res;
} MySQLResultWrapper;

static inline MySQLWrapper *mysql_wrapper_new(MYSQL *mysql);
static inline MySQLResultWrapper *mysql_result_wrapper_new(MYSQL_RES *res);
static void mysql_wrapper_deinit(HkUserdata *udata);
static void mysql_result_wrapper_deinit(HkUserdata *udata);
static void connect_call(HkVM *vm, HkValue *args);
static void close_call(HkVM *vm, HkValue *args);
static void ping_call(HkVM *vm, HkValue *args);
static void error_call(HkVM *vm, HkValue *args);
static void select_db_call(HkVM *vm, HkValue *args);
static void query_call(HkVM *vm, HkValue *args);
static void fetch_row_call(HkVM *vm, HkValue *args);
static void affected_rows_call(HkVM *vm, HkValue *args);

static inline MySQLWrapper *mysql_wrapper_new(MYSQL *mysql)
{
  MySQLWrapper *wrapper = (MySQLWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, mysql_wrapper_deinit);
  wrapper->mysql = mysql;
  return wrapper;
}

static inline MySQLResultWrapper *mysql_result_wrapper_new(MYSQL_RES *res)
{
  MySQLResultWrapper *wrapper = (MySQLResultWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, mysql_result_wrapper_deinit);
  wrapper->res = res;
  return wrapper;
}

static void mysql_wrapper_deinit(HkUserdata *udata)
{
  MYSQL *mysql = ((MySQLWrapper *) udata)->mysql;
  if (!mysql)
    return;
  mysql_close(mysql);
  mysql_library_end();
}

static void mysql_result_wrapper_deinit(HkUserdata *udata)
{
  MYSQL_RES *res = ((MySQLResultWrapper *) udata)->res;
  if (!res)
    return;
  mysql_free_result(res);
}

static void connect_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_types(vm, args, 1, 2, (HkType[]) { HK_TYPE_NIL, HK_TYPE_STRING });
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_types(vm, args, 2, 2, (HkType[]) { HK_TYPE_NIL, HK_TYPE_NUMBER });
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_types(vm, args, 3, 2, (HkType[]) { HK_TYPE_NIL, HK_TYPE_STRING });
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_types(vm, args, 4, 2, (HkType[]) { HK_TYPE_NIL, HK_TYPE_STRING });
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_types(vm, args, 5, 2, (HkType[]) { HK_TYPE_NIL, HK_TYPE_STRING });
  hk_return_if_not_ok(vm);
  if (mysql_library_init(0, NULL, NULL))
  {
    hk_vm_runtime_error(vm, "cannot initialize MySQL client library");
    return;
  }
  MYSQL *mysql = NULL;
  mysql = mysql_init(mysql);
  const char *host = hk_is_nil(args[1]) ? NULL : hk_as_string(args[1])->chars;
  int port = hk_is_nil(args[2]) ? 0 : (int) hk_as_number(args[2]);
  const char *username = hk_is_nil(args[3]) ? NULL : hk_as_string(args[3])->chars;
  const char *password = hk_is_nil(args[4]) ? NULL : hk_as_string(args[4])->chars;
  const char *database = hk_is_nil(args[5]) ? NULL : hk_as_string(args[5])->chars;
  HkArray *result = hk_array_new_with_capacity(2);
  result->length = 2;
  if (!mysql_real_connect(mysql, host, username, password, database, port, NULL, CLIENT_FOUND_ROWS))
  {
    HkString *err = hk_string_from_chars(-1, mysql_error(mysql));
    mysql_close(mysql);
    mysql_library_end();
    hk_incr_ref(err);
    result->elements[0] = hk_nil_value();
    result->elements[1] = hk_string_value(err);
    hk_vm_push_array(vm, result);
    return;
  }
  HkUserdata *udata = (HkUserdata *) mysql_wrapper_new(mysql);
  hk_incr_ref(udata);
  result->elements[0] = hk_userdata_value(udata);
  result->elements[1] = hk_nil_value();
  hk_vm_push_array(vm, result);
}

static void close_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  MySQLWrapper *wrapper = (MySQLWrapper *) hk_as_userdata(args[1]);
  bool result = false;
  if (wrapper->mysql)
  {
    mysql_close(wrapper->mysql);
    mysql_library_end();
    wrapper->mysql = NULL;
    result = true;
  }
  hk_vm_push_bool(vm, result);
}

static void ping_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  MYSQL *mysql = ((MySQLWrapper *) hk_as_userdata(args[1]))->mysql;
  hk_vm_push_bool(vm, !mysql_ping(mysql));
}

static void error_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  MYSQL *mysql = ((MySQLWrapper *) hk_as_userdata(args[1]))->mysql;
  hk_vm_push_string_from_chars(vm, -1, mysql_error(mysql));
}

static void select_db_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  MYSQL *mysql = ((MySQLWrapper *) hk_as_userdata(args[1]))->mysql;
  const char *database = hk_as_string(args[2])->chars;
  hk_vm_push_bool(vm, !mysql_select_db(mysql, database));
}

static void query_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  MYSQL *mysql = ((MySQLWrapper *) hk_as_userdata(args[1]))->mysql;
  const char *query = hk_as_string(args[2])->chars;
  HkArray *result = hk_array_new_with_capacity(2);
  result->length = 2;
  if (mysql_query(mysql, query))
  {
    HkString *err = hk_string_from_chars(-1, mysql_error(mysql));
    hk_incr_ref(err);
    result->elements[0] = hk_nil_value();
    result->elements[1] = hk_string_value(err);
    hk_vm_push_array(vm, result);
    return;
  }
  MYSQL_RES *mysql_res = mysql_store_result(mysql);
  if (!mysql_res)
  {
    result->elements[0] = hk_nil_value();
    result->elements[1] = hk_nil_value();
    hk_vm_push_array(vm, result);
    return;
  }
  HkUserdata *udata = (HkUserdata *) mysql_result_wrapper_new(mysql_res);
  hk_incr_ref(udata);
  result->elements[0] = hk_userdata_value(udata);
  result->elements[1] = hk_nil_value();
  hk_vm_push_array(vm, result);
}

static void fetch_row_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  MYSQL_RES *res = ((MySQLResultWrapper *) hk_as_userdata(args[1]))->res;
  MYSQL_FIELD *fields = mysql_fetch_fields(res);
  MYSQL_ROW row = mysql_fetch_row(res);
  if (!row)
  {
    hk_vm_push_nil(vm);
    return;
  }
  int numFields = mysql_num_fields(res);
  HkArray *arr = hk_array_new_with_capacity(numFields);
  for (int i = 0; i < numFields; ++i)
  {
    char *chars = row[i];
    if (!chars)
    {
      hk_array_inplace_append_element(arr, hk_nil_value());
      break;
    }
    HkValue elem = hk_nil_value();
    switch (fields[i].type)
    {
    case MYSQL_TYPE_NULL:
      break;
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_NEWDECIMAL:
      elem = hk_number_value(atof(chars));
      break;
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_TIMESTAMP2:
    case MYSQL_TYPE_JSON:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_GEOMETRY:
      elem = hk_string_value(hk_string_from_chars(-1, chars));
      break;
    default:
      break;
    }
    hk_array_inplace_append_element(arr, elem);
  }
  hk_vm_push_array(vm, arr);
}

static void affected_rows_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  MYSQL *mysql = ((MySQLWrapper *) hk_as_userdata(args[1]))->mysql;
  hk_vm_push_number(vm, (double) mysql_affected_rows(mysql));
}

HK_LOAD_MODULE_HANDLER(mysql)
{
  hk_vm_push_string_from_chars(vm, -1, "mysql");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "connect");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "connect", 5, connect_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "close");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "close", 1, close_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "ping");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "ping", 1, ping_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "error");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "error", 1, error_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "select_db");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "select_db", 2, select_db_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "query");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "query", 2, query_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "fetch_row");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "fetch_row", 1, fetch_row_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "affected_rows");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "affected_rows", 1, affected_rows_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 8);
}
