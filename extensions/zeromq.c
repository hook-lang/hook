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
} zeromq_context_wrapper_t;

typedef struct
{
  HK_USERDATA_HEADER
  void *sock;
} zeromq_socket_wrapper_t;

static inline zeromq_context_wrapper_t *zeromq_context_wrapper_new(void *ctx);
static inline zeromq_socket_wrapper_t *zeromq_socket_wrapper_new(void *sock);
static void zeromq_context_wrapper_deinit(hk_userdata_t *udata);
static void zeromq_socket_wrapper_deinit(hk_userdata_t *udata);
static int32_t new_context_call(hk_state_t *state, hk_value_t *args);
static int32_t new_socket_call(hk_state_t *state, hk_value_t *args);
static int32_t close_call(hk_state_t *state, hk_value_t *args);
static int32_t connect_call(hk_state_t *state, hk_value_t *args);
static int32_t bind_call(hk_state_t *state, hk_value_t *args);
static int32_t send_call(hk_state_t *state, hk_value_t *args);
static int32_t recv_call(hk_state_t *state, hk_value_t *args);

static inline zeromq_context_wrapper_t *zeromq_context_wrapper_new(void *ctx)
{
  zeromq_context_wrapper_t *wrapper = (zeromq_context_wrapper_t *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((hk_userdata_t *) wrapper, &zeromq_context_wrapper_deinit);
  wrapper->ctx = ctx;
  return wrapper;
}

static inline zeromq_socket_wrapper_t *zeromq_socket_wrapper_new(void *sock)
{
  zeromq_socket_wrapper_t *wrapper = (zeromq_socket_wrapper_t *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((hk_userdata_t *) wrapper, &zeromq_socket_wrapper_deinit);
  wrapper->sock = sock;
  return wrapper;
}

static void zeromq_context_wrapper_deinit(hk_userdata_t *udata)
{
  void *ctx = ((zeromq_context_wrapper_t *) udata)->ctx;
  hk_assert(ctx, "context is NULL");
  (void) zmq_ctx_destroy(ctx);
}

static void zeromq_socket_wrapper_deinit(hk_userdata_t *udata)
{
  void *sock = ((zeromq_socket_wrapper_t *) udata)->sock;
  if (!sock)
    return;
  (void) zmq_close(sock);
}

static int32_t new_context_call(hk_state_t *state, hk_value_t *args)
{
  (void) args;
  void *ctx = zmq_ctx_new();
  if (!ctx)
    return hk_state_push_nil(state);
  zeromq_context_wrapper_t *wrapper = zeromq_context_wrapper_new(ctx);
  return hk_state_push_userdata(state, (hk_userdata_t *) wrapper);
}

static int32_t new_socket_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  zeromq_context_wrapper_t *wrapper = (zeromq_context_wrapper_t *) hk_as_userdata(args[1]);
  int32_t type = (int32_t) hk_as_number(args[2]);
  void *ctx = wrapper->ctx;
  void *sock = zmq_socket(ctx, type);
  if (!sock)
    return hk_state_push_nil(state);
  return hk_state_push_userdata(state, (hk_userdata_t *) zeromq_socket_wrapper_new(sock));
}

static int32_t close_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  zeromq_socket_wrapper_t *wrapper = (zeromq_socket_wrapper_t *) hk_as_userdata(args[1]);
  void *sock = wrapper->sock;
  if (!sock)
  {
    (void) zmq_close(sock);
    wrapper->sock = NULL;
  }
  return hk_state_push_nil(state);
}

static int32_t connect_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  zeromq_socket_wrapper_t *wrapper = (zeromq_socket_wrapper_t *) hk_as_userdata(args[1]);
  hk_string_t *host = hk_as_string(args[2]);
  void *sock = wrapper->sock;
  if (zmq_connect(sock, host->chars))
  {
    hk_runtime_error("cannot connect to address '%.*s'", host->length, host->chars);
    return HK_STATUS_ERROR;
  }
  return hk_state_push_nil(state);
}

static int32_t bind_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  zeromq_socket_wrapper_t *wrapper = (zeromq_socket_wrapper_t *) hk_as_userdata(args[1]);
  hk_string_t *host = hk_as_string(args[2]);
  void *sock = wrapper->sock;
  if (zmq_bind(sock, host->chars))
  {
    hk_runtime_error("cannot bind to address '%.*s'", host->length, host->chars);
    return HK_STATUS_ERROR;
  }
  return hk_state_push_nil(state);
}

static int32_t send_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  zeromq_socket_wrapper_t *wrapper = (zeromq_socket_wrapper_t *) hk_as_userdata(args[1]);
  hk_string_t *str = hk_as_string(args[2]);
  int32_t flags = (int32_t) hk_as_number(args[3]);
  int32_t length = (int32_t) zmq_send(wrapper->sock, str->chars, str->length, flags);
  return hk_state_push_number(state, length);
}

static int32_t recv_call(hk_state_t *state, hk_value_t *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 3) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  zeromq_socket_wrapper_t *wrapper = (zeromq_socket_wrapper_t *) hk_as_userdata(args[1]);
  int32_t size = (int32_t) hk_as_number(args[2]);
  int32_t flags = (int32_t) hk_as_number(args[3]);
  hk_string_t *str = hk_string_new_with_capacity(size);
  int32_t length = (int32_t) zmq_recv(wrapper->sock, str->chars, size, flags);
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
