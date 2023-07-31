//
// The Hook Programming Language
// zeromq.c
//

#include "zeromq.h"
#include <stdlib.h>
#include <zmq.h>
#include <hook/memory.h>
#include <hook/check.h>
#include <hook/status.h>
#include <hook/error.h>

typedef struct
{
  HK_USERDATA_HEADER
  void *ctx;
} ZeroMQContextWrapper;

typedef struct
{
  HK_USERDATA_HEADER
  void *sock;
} ZeroMQWocketWrapper;

static inline ZeroMQContextWrapper *zeromq_context_wrapper_new(void *ctx);
static inline ZeroMQWocketWrapper *zeromq_socket_wrapper_new(void *sock);
static void zeromq_context_wrapper_deinit(HkUserdata *udata);
static void zeromq_socket_wrapper_deinit(HkUserdata *udata);
static int new_context_call(HkState *state, HkValue *args);
static int new_socket_call(HkState *state, HkValue *args);
static int close_call(HkState *state, HkValue *args);
static int connect_call(HkState *state, HkValue *args);
static int bind_call(HkState *state, HkValue *args);
static int send_call(HkState *state, HkValue *args);
static int recv_call(HkState *state, HkValue *args);

static inline ZeroMQContextWrapper *zeromq_context_wrapper_new(void *ctx)
{
  ZeroMQContextWrapper *wrapper = (ZeroMQContextWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &zeromq_context_wrapper_deinit);
  wrapper->ctx = ctx;
  return wrapper;
}

static inline ZeroMQWocketWrapper *zeromq_socket_wrapper_new(void *sock)
{
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &zeromq_socket_wrapper_deinit);
  wrapper->sock = sock;
  return wrapper;
}

static void zeromq_context_wrapper_deinit(HkUserdata *udata)
{
  void *ctx = ((ZeroMQContextWrapper *) udata)->ctx;
  hk_assert(ctx, "context is NULL");
  (void) zmq_ctx_destroy(ctx);
}

static void zeromq_socket_wrapper_deinit(HkUserdata *udata)
{
  void *sock = ((ZeroMQWocketWrapper *) udata)->sock;
  if (!sock)
    return;
  (void) zmq_close(sock);
}

static int new_context_call(HkState *state, HkValue *args)
{
  (void) args;
  void *ctx = zmq_ctx_new();
  if (!ctx)
    return hk_state_push_nil(state);
  ZeroMQContextWrapper *wrapper = zeromq_context_wrapper_new(ctx);
  return hk_state_push_userdata(state, (HkUserdata *) wrapper);
}

static int new_socket_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  ZeroMQContextWrapper *wrapper = (ZeroMQContextWrapper *) hk_as_userdata(args[1]);
  int type = (int) hk_as_number(args[2]);
  void *ctx = wrapper->ctx;
  void *sock = zmq_socket(ctx, type);
  if (!sock)
    return hk_state_push_nil(state);
  return hk_state_push_userdata(state, (HkUserdata *) zeromq_socket_wrapper_new(sock));
}

static int close_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_as_userdata(args[1]);
  void *sock = wrapper->sock;
  if (!sock)
  {
    (void) zmq_close(sock);
    wrapper->sock = NULL;
  }
  return hk_state_push_nil(state);
}

static int connect_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_as_userdata(args[1]);
  HkString *host = hk_as_string(args[2]);
  void *sock = wrapper->sock;
  if (zmq_connect(sock, host->chars))
  {
    hk_runtime_error("cannot connect to address '%.*s'", host->length, host->chars);
    return HK_STATUS_ERROR;
  }
  return hk_state_push_nil(state);
}

static int bind_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_as_userdata(args[1]);
  HkString *host = hk_as_string(args[2]);
  void *sock = wrapper->sock;
  if (zmq_bind(sock, host->chars))
  {
    hk_runtime_error("cannot bind to address '%.*s'", host->length, host->chars);
    return HK_STATUS_ERROR;
  }
  return hk_state_push_nil(state);
}

static int send_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_as_userdata(args[1]);
  HkString *str = hk_as_string(args[2]);
  int flags = (int) hk_as_number(args[3]);
  int length = (int) zmq_send(wrapper->sock, str->chars, str->length, flags);
  return hk_state_push_number(state, length);
}

static int recv_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_as_userdata(args[1]);
  int size = (int) hk_as_number(args[2]);
  int flags = (int) hk_as_number(args[3]);
  HkString *str = hk_string_new_with_capacity(size);
  int length = (int) zmq_recv(wrapper->sock, str->chars, size, flags);
  if (!length)
    return hk_state_push_nil(state);
  str->length = length;
  return hk_state_push_string(state, str);
}

HK_LOAD_FN(zeromq)
{
  if (hk_state_push_string_from_chars(state, -1, "zeromq") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "ZMQ_REQ") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, ZMQ_REQ) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "ZMQ_REP") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_number(state, ZMQ_REP) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "new_context") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "new_context", 0, &new_context_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "new_socket") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "new_socket", 2, &new_socket_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "close") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "close", 1, &close_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "connect") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "connect", 2, &connect_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "bind") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "bind", 2, &bind_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "send") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "send", 3, &send_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "recv") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "recv", 3, &recv_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 9);
}
