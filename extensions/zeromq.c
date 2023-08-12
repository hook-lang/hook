//
// The Hook Programming Language
// zeromq.c
//

#include "zeromq.h"
#include <stdlib.h>
#include <zmq.h>

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
static void new_context_call(HkState *state, HkValue *args);
static void new_socket_call(HkState *state, HkValue *args);
static void close_call(HkState *state, HkValue *args);
static void connect_call(HkState *state, HkValue *args);
static void bind_call(HkState *state, HkValue *args);
static void send_call(HkState *state, HkValue *args);
static void recv_call(HkState *state, HkValue *args);

static inline ZeroMQContextWrapper *zeromq_context_wrapper_new(void *ctx)
{
  ZeroMQContextWrapper *wrapper = (ZeroMQContextWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, zeromq_context_wrapper_deinit);
  wrapper->ctx = ctx;
  return wrapper;
}

static inline ZeroMQWocketWrapper *zeromq_socket_wrapper_new(void *sock)
{
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, zeromq_socket_wrapper_deinit);
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

static void new_context_call(HkState *state, HkValue *args)
{
  (void) args;
  void *ctx = zmq_ctx_new();
  if (!ctx)
  {
    hk_state_push_nil(state);
    return;
  }
  ZeroMQContextWrapper *wrapper = zeromq_context_wrapper_new(ctx);
  hk_state_push_userdata(state, (HkUserdata *) wrapper);
}

static void new_socket_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  ZeroMQContextWrapper *wrapper = (ZeroMQContextWrapper *) hk_as_userdata(args[1]);
  int type = (int) hk_as_number(args[2]);
  void *ctx = wrapper->ctx;
  void *sock = zmq_socket(ctx, type);
  if (!sock)
  {
    hk_state_push_nil(state);
    return;
  }
  hk_state_push_userdata(state, (HkUserdata *) zeromq_socket_wrapper_new(sock));
}

static void close_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_as_userdata(args[1]);
  void *sock = wrapper->sock;
  if (!sock)
  {
    (void) zmq_close(sock);
    wrapper->sock = NULL;
  }
  hk_state_push_nil(state);
}

static void connect_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_as_userdata(args[1]);
  HkString *host = hk_as_string(args[2]);
  void *sock = wrapper->sock;
  if (zmq_connect(sock, host->chars))
  {
    hk_state_runtime_error(state, "cannot connect to address '%.*s'", host->length, host->chars);
    return;
  }
  hk_state_push_nil(state);
}

static void bind_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_as_userdata(args[1]);
  HkString *host = hk_as_string(args[2]);
  void *sock = wrapper->sock;
  if (zmq_bind(sock, host->chars))
  {
    hk_state_runtime_error(state, "cannot bind to address '%.*s'", host->length, host->chars);
    return;
  }
  hk_state_push_nil(state);
}

static void send_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_string(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_as_userdata(args[1]);
  HkString *str = hk_as_string(args[2]);
  int flags = (int) hk_as_number(args[3]);
  int length = (int) zmq_send(wrapper->sock, str->chars, str->length, flags);
  hk_state_push_number(state, length);
}

static void recv_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_userdata(state, args, 1);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 2);
  hk_return_if_not_ok(state);
  hk_state_check_argument_int(state, args, 3);
  hk_return_if_not_ok(state);
  ZeroMQWocketWrapper *wrapper = (ZeroMQWocketWrapper *) hk_as_userdata(args[1]);
  int size = (int) hk_as_number(args[2]);
  int flags = (int) hk_as_number(args[3]);
  HkString *str = hk_string_new_with_capacity(size);
  int length = (int) zmq_recv(wrapper->sock, str->chars, size, flags);
  if (!length)
  {
    hk_state_push_nil(state);
    return;
  }
  str->length = length;
  hk_state_push_string(state, str);
}

HK_LOAD_MODULE_HANDLER(zeromq)
{
  hk_state_push_string_from_chars(state, -1, "zeromq");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "ZMQ_REQ");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, ZMQ_REQ);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "ZMQ_REP");
  hk_return_if_not_ok(state);
  hk_state_push_number(state, ZMQ_REP);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new_context");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new_context", 0, new_context_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new_socket");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new_socket", 2, new_socket_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "close");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "close", 1, close_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "connect");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "connect", 2, connect_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "bind");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "bind", 2, bind_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "send");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "send", 3, send_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "recv");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "recv", 3, recv_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 9);
}
