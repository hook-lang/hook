//
// The Hook Programming Language
// mysql.c
//

#include "mysql.h"
#include <mysql/mysql.h>
#include <assert.h>
#include "hk_memory.h"
#include "hk_status.h"
#include "hk_error.h"

typedef struct
{
  HK_USERDATA_HEADER
  MYSQL *my;
} mysql_t;

typedef struct
{
  HK_USERDATA_HEADER
  MYSQL_RES *res;
} mysql_result_t;

static inline mysql_t *mysql_new(MYSQL *my);
static inline mysql_result_t *mysql_result_new(MYSQL_RES *res);
static void mysql_deinit(hk_userdata_t *udata);
static void mysql_result_deinit(hk_userdata_t *udata);
static int32_t connect_call(hk_vm_t *vm, hk_value_t *args);
static int32_t close_call(hk_vm_t *vm, hk_value_t *args);
static int32_t ping_call(hk_vm_t *vm, hk_value_t *args);
static int32_t error_call(hk_vm_t *vm, hk_value_t *args);
static int32_t select_db_call(hk_vm_t *vm, hk_value_t *args);
static int32_t query_call(hk_vm_t *vm, hk_value_t *args);
static int32_t fetch_row_call(hk_vm_t *vm, hk_value_t *args);
static int32_t affected_rows_call(hk_vm_t *vm, hk_value_t *args);

static inline mysql_t *mysql_new(MYSQL *my)
{
  mysql_t *mysql = (mysql_t *) hk_allocate(sizeof(*mysql));
  hk_userdata_init((hk_userdata_t *) mysql, &mysql_deinit);
  mysql->my = my;
  return mysql;
}

static inline mysql_result_t *mysql_result_new(MYSQL_RES *res)
{
  mysql_result_t *mysql_result = (mysql_result_t *) hk_allocate(sizeof(*mysql_result));
  hk_userdata_init((hk_userdata_t *) mysql_result, &mysql_result_deinit);
  mysql_result->res = res;
  return mysql_result;
}

static void mysql_deinit(hk_userdata_t *udata)
{
  mysql_t *mysql = (mysql_t *) udata;
  if (!mysql->my)
    return;
  mysql_close(mysql->my);
  mysql_library_end();
}

static void mysql_result_deinit(hk_userdata_t *udata)
{
  mysql_result_t *mysql_result = (mysql_result_t *) udata;
  if (!mysql_result->res)
    return;
  mysql_free_result(mysql_result->res);
}

static int32_t connect_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_types(args, 1, 2, (int32_t[]) {HK_TYPE_NIL, HK_TYPE_STRING}) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_types(args, 2, 2, (int32_t[]) {HK_TYPE_NIL, HK_TYPE_FLOAT}) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_types(args, 3, 2, (int32_t[]) {HK_TYPE_NIL, HK_TYPE_STRING}) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_types(args, 4, 2, (int32_t[]) {HK_TYPE_NIL, HK_TYPE_STRING}) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_types(args, 5, 2, (int32_t[]) {HK_TYPE_NIL, HK_TYPE_STRING}) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (mysql_library_init(0, NULL, NULL))
  {
    hk_runtime_error("cannot initialize MySQL client library");
    return HK_STATUS_ERROR;
  }
  MYSQL *my = NULL;
  my = mysql_init(my);
  const char *host = hk_is_nil(args[1]) ? NULL : hk_as_string(args[1])->chars;
  int32_t port = hk_is_nil(args[2]) ? 0 : (int32_t) hk_as_float(args[2]);
  const char *username = hk_is_nil(args[3]) ? NULL : hk_as_string(args[3])->chars;
  const char *password = hk_is_nil(args[4]) ? NULL : hk_as_string(args[4])->chars;
  const char *database = hk_is_nil(args[5]) ? NULL : hk_as_string(args[5])->chars;
  hk_array_t *result = hk_array_new_with_capacity(2);
  result->length = 2;
  if (!mysql_real_connect(my, host, username, password, database, port, NULL, CLIENT_FOUND_ROWS))
  {
    hk_string_t *err = hk_string_from_chars(-1, mysql_error(my));
    mysql_close(my);
    mysql_library_end();
    hk_incr_ref(err);
    result->elements[0] = HK_NIL_VALUE;
    result->elements[1] = hk_string_value(err);
    return hk_vm_push_array(vm, result);
  }
  hk_userdata_t *udata = (hk_userdata_t *) mysql_new(my);
  hk_incr_ref(udata);
  result->elements[0] = hk_userdata_value(udata);
  result->elements[1] = HK_NIL_VALUE;
  return hk_vm_push_array(vm, result);
}

static int32_t close_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  mysql_t *mysql = (mysql_t *) hk_as_userdata(args[1]);
  bool result = false;
  if (mysql->my)
  {
    mysql_close(mysql->my);
    mysql_library_end();
    mysql->my = NULL;
    result = true;
  }
  return hk_vm_push_bool(vm, result);
}

static int32_t ping_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  MYSQL *my = ((mysql_t *) hk_as_userdata(args[1]))->my;
  return hk_vm_push_bool(vm, !mysql_ping(my));
}

static int32_t error_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  MYSQL *my = ((mysql_t *) hk_as_userdata(args[1]))->my;
  return hk_vm_push_string_from_chars(vm, -1, mysql_error(my));
}

static int32_t select_db_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  MYSQL *my = ((mysql_t *) hk_as_userdata(args[1]))->my;
  const char *database = hk_as_string(args[2])->chars;
  return hk_vm_push_bool(vm, !mysql_select_db(my, database));
}

static int32_t query_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  MYSQL *my = ((mysql_t *) hk_as_userdata(args[1]))->my;
  const char *query = hk_as_string(args[2])->chars;
  hk_array_t *result = hk_array_new_with_capacity(2);
  result->length = 2;
  if (mysql_query(my, query))
  {
    hk_string_t *err = hk_string_from_chars(-1, mysql_error(my));
    hk_incr_ref(err);
    result->elements[0] = HK_NIL_VALUE;
    result->elements[1] = hk_string_value(err);
    return hk_vm_push_array(vm, result);
  }
  MYSQL_RES *res = mysql_store_result(my);
  if (!res)
  {
    result->elements[0] = HK_NIL_VALUE;
    result->elements[1] = HK_NIL_VALUE;
    return hk_vm_push_array(vm, result);
  }
  hk_userdata_t *udata = (hk_userdata_t *) mysql_result_new(res);
  hk_incr_ref(udata);
  result->elements[0] = hk_userdata_value(udata);
  result->elements[1] = HK_NIL_VALUE;
  return hk_vm_push_array(vm, result);
}

static int32_t fetch_row_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  MYSQL_RES *res = ((mysql_result_t *) hk_as_userdata(args[1]))->res;
  MYSQL_FIELD *fields = mysql_fetch_fields(res);
  MYSQL_ROW row = mysql_fetch_row(res);
  if (!row)
    return hk_vm_push_nil(vm);
  int32_t num_fields = mysql_num_fields(res);
  hk_array_t *arr = hk_array_new_with_capacity(num_fields);
  for (int32_t i = 0; i < num_fields; ++i)
  {
    char *chars = row[i];
    if (!chars)
    {
      hk_array_inplace_add_element(arr, HK_NIL_VALUE);
      break;
    }
    hk_value_t elem = HK_NIL_VALUE;
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
      elem = hk_float_value(atof(chars));
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
  return hk_vm_push_array(vm, arr);
}

static int32_t affected_rows_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  MYSQL *my = ((mysql_t *) hk_as_userdata(args[1]))->my;
  return hk_vm_push_float(vm, (double) mysql_affected_rows(my));
}

HK_LOAD_FN(mysql)
{
  if (hk_vm_push_string_from_chars(vm, -1, "mysql") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "connect") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "connect", 5, &connect_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "close") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "close", 1, &close_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "ping") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "ping", 1, &ping_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "error") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "error", 1, &error_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "select_db") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "select_db", 2, &select_db_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "query") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "query", 2, &query_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "fetch_row") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "fetch_row", 1, &fetch_row_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "affected_rows") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "affected_rows", 1, &affected_rows_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 8);
}
