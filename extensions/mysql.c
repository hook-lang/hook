//
// The Hook Programming Language
// mysql.c
//

#include "mysql.h"
#include <mysql/mysql.h>
#include <assert.h>
#include <hook/memory.h>

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
static void connect_call(HkState *state, HkValue *args);
static void close_call(HkState *state, HkValue *args);
static void ping_call(HkState *state, HkValue *args);
static void error_call(HkState *state, HkValue *args);
static void select_db_call(HkState *state, HkValue *args);
static void query_call(HkState *state, HkValue *args);
static void fetch_row_call(HkState *state, HkValue *args);
static void affected_rows_call(HkState *state, HkValue *args);

static inline MySQLWrapper *mysql_wrapper_new(MYSQL *mysql)
{
  MySQLWrapper *wrapper = (MySQLWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &mysql_wrapper_deinit);
  wrapper->mysql = mysql;
  return wrapper;
}

static inline MySQLResultWrapper *mysql_result_wrapper_new(MYSQL_RES *res)
{
  MySQLResultWrapper *wrapper = (MySQLResultWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &mysql_result_wrapper_deinit);
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

static void connect_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_types(state, args, 1, 2, (HkType[]) {HK_TYPE_NIL, HK_TYPE_STRING});
  hk_return_if_not_ok(state);
  hk_state_check_argument_types(state, args, 2, 2, (HkType[]) {HK_TYPE_NIL, HK_TYPE_NUMBER});
  hk_return_if_not_ok(state);
  hk_state_check_argument_types(state, args, 3, 2, (HkType[]) {HK_TYPE_NIL, HK_TYPE_STRING});
  hk_return_if_not_ok(state);
  hk_state_check_argument_types(state, args, 4, 2, (HkType[]) {HK_TYPE_NIL, HK_TYPE_STRING});
  hk_return_if_not_ok(state);
  hk_state_check_argument_types(state, args, 5, 2, (HkType[]) {HK_TYPE_NIL, HK_TYPE_STRING});
  hk_return_if_not_ok(state);
  if (mysql_library_init(0, NULL, NULL))
  {
    hk_state_runtime_error(state, "cannot initialize MySQL client library");
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
    result->elements[0] = HK_NIL_VALUE;
    result->elements[1] = hk_string_value(err);
    hk_state_push_array(state, result);
    return;
  }
  HkUserdata *udata = (HkUserdata *) mysql_wrapper_new(mysql);
  hk_incr_ref(udata);
  result->elements[0] = hk_userdata_value(udata);
  result->elements[1] = HK_NIL_VALUE;
  hk_state_push_array(state, result);
}

static void close_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  MySQLWrapper *wrapper = (MySQLWrapper *) hk_as_userdata(args[1]);
  bool result = false;
  if (wrapper->mysql)
  {
    mysql_close(wrapper->mysql);
    mysql_library_end();
    wrapper->mysql = NULL;
    result = true;
  }
  hk_state_push_bool(state, result);
}

static void ping_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  MYSQL *mysql = ((MySQLWrapper *) hk_as_userdata(args[1]))->mysql;
  hk_state_push_bool(state, !mysql_ping(mysql));
}

static void error_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  MYSQL *mysql = ((MySQLWrapper *) hk_as_userdata(args[1]))->mysql;
  hk_state_push_string_from_chars(state, -1, mysql_error(mysql));
}

static void select_db_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  MYSQL *mysql = ((MySQLWrapper *) hk_as_userdata(args[1]))->mysql;
  const char *database = hk_as_string(args[2])->chars;
  hk_state_push_bool(state, !mysql_select_db(mysql, database));
}

static void query_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  MYSQL *mysql = ((MySQLWrapper *) hk_as_userdata(args[1]))->mysql;
  const char *query = hk_as_string(args[2])->chars;
  HkArray *result = hk_array_new_with_capacity(2);
  result->length = 2;
  if (mysql_query(mysql, query))
  {
    HkString *err = hk_string_from_chars(-1, mysql_error(mysql));
    hk_incr_ref(err);
    result->elements[0] = HK_NIL_VALUE;
    result->elements[1] = hk_string_value(err);
    hk_state_push_array(state, result);
    return;
  }
  MYSQL_RES *mysql_res = mysql_store_result(mysql);
  if (!mysql_res)
  {
    result->elements[0] = HK_NIL_VALUE;
    result->elements[1] = HK_NIL_VALUE;
    hk_state_push_array(state, result);
    return;
  }
  HkUserdata *udata = (HkUserdata *) mysql_result_wrapper_new(mysql_res);
  hk_incr_ref(udata);
  result->elements[0] = hk_userdata_value(udata);
  result->elements[1] = HK_NIL_VALUE;
  hk_state_push_array(state, result);
}

static void fetch_row_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  MYSQL_RES *res = ((MySQLResultWrapper *) hk_as_userdata(args[1]))->res;
  MYSQL_FIELD *fields = mysql_fetch_fields(res);
  MYSQL_ROW row = mysql_fetch_row(res);
  if (!row)
  {
    hk_state_push_nil(state);
    return;
  }
  int numFields = mysql_num_fields(res);
  HkArray *arr = hk_array_new_with_capacity(numFields);
  for (int i = 0; i < numFields; ++i)
  {
    char *chars = row[i];
    if (!chars)
    {
      hk_array_inplace_add_element(arr, HK_NIL_VALUE);
      break;
    }
    HkValue elem = HK_NIL_VALUE;
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
    hk_array_inplace_add_element(arr, elem);
  }
  hk_state_push_array(state, arr);
}

static void affected_rows_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  MYSQL *mysql = ((MySQLWrapper *) hk_as_userdata(args[1]))->mysql;
  hk_state_push_number(state, (double) mysql_affected_rows(mysql));
}

HK_LOAD_MODULE_HANDLER(mysql)
{
  hk_state_push_string_from_chars(state, -1, "mysql");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "connect");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "connect", 5, &connect_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "close");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "close", 1, &close_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "ping");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "ping", 1, &ping_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "error");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "error", 1, &error_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "select_db");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "select_db", 2, &select_db_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "query");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "query", 2, &query_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "fetch_row");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "fetch_row", 1, &fetch_row_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "affected_rows");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "affected_rows", 1, &affected_rows_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 8);
}
