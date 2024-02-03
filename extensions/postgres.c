//
// The Hook Programming Language
// postgres.c
//

#include "postgres.h"
#include <libpq-fe.h>

typedef struct
{
  HK_USERDATA_HEADER
  PGconn *conn;
} PGConnectionUserdata;

static inline PGConnectionUserdata *pgconn_userdata_new(PGconn *conn);
static void pgconn_userdata_deinit(HkUserdata *udata);
static void connect_call(HkState *state, HkValue *args);
static void close_call(HkState *state, HkValue *args);

static inline PGConnectionUserdata *pgconn_userdata_new(PGconn *conn)
{
  PGConnectionUserdata *udata = (PGConnectionUserdata *) hk_allocate(sizeof(*udata));
  hk_userdata_init((HkUserdata *) udata, pgconn_userdata_deinit);
  udata->conn = conn;
  return udata;
}

static void pgconn_userdata_deinit(HkUserdata *udata)
{
  PGconn *conn = ((PGConnectionUserdata *) udata)->conn;
  if (!conn)
    return;
  PQfinish(conn);
}

static void connect_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  const char *connInfo = (const char *) (hk_as_string(args[1]))->chars;
  PGconn *conn = PQconnectdb(connInfo);
  HkArray *result = hk_array_new_with_capacity(2);
  result->length = 2;
  if (PQstatus(conn) != CONNECTION_OK)
  {
    HkString *err = hk_string_from_chars(-1, PQerrorMessage(conn));
    PQfinish(conn);
    hk_incr_ref(err);
    result->elements[0] = HK_NIL_VALUE;
    result->elements[1] = hk_string_value(err);
    hk_state_push_array(state, result);
    return;
  }
  HkUserdata *udata = (HkUserdata *) pgconn_userdata_new(conn);
  hk_incr_ref(udata);
  result->elements[0] = hk_userdata_value(udata);
  result->elements[1] = HK_NIL_VALUE;
  hk_state_push_array(state, result);
}

static void close_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  PGConnectionUserdata *udata = (PGConnectionUserdata *) hk_as_userdata(args[1]);
  bool result = false;
  if (udata->conn)
  {
    PQfinish(udata->conn);
    udata->conn = NULL;
    result = true;
  }
  hk_state_push_bool(state, result);
}

HK_LOAD_MODULE_HANDLER(postgres)
{
  hk_state_push_string_from_chars(state, -1, "postgres");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "connect");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "connect", 1, connect_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "close");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "close", 1, close_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 2);
}
