//
// The Hook Programming Language
// polling.c
//

#include "polling.h"

#ifdef _WIN32
  #include "deps/wepoll.h"
#endif

#ifdef __linux__
  #include <sys/epoll.h>
#endif

#ifdef __APPLE__
  #include <sys/event.h>
#endif

#ifndef _WIN32
  #define HANDLER int
#endif

typedef struct
{
  HK_USERDATA_HEADER
  HANDLER handler;
} HandlerWrapper;

static inline HandlerWrapper *handler_wrapper_new(HANDLER handler);
static void handler_wrapper_deinit(HkUserdata *udata);
static void new_call(HkState *state, HkValue *args);
static void close_call(HkState *state, HkValue *args);
static void add_call(HkState *state, HkValue *args);
static void modify_call(HkState *state, HkValue *args);
static void delete_call(HkState *state, HkValue *args);
static void wait_call(HkState *state, HkValue *args);

static inline HandlerWrapper *handler_wrapper_new(HANDLER handler)
{
  HandlerWrapper *wrapper = (HandlerWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, handler_wrapper_deinit);
  wrapper->handler = handler;
  return wrapper;
}

static void handler_wrapper_deinit(HkUserdata *udata)
{
  // TODO: Implement this function.
  (void) udata;
}

static void new_call(HkState *state, HkValue *args)
{
  // TODO: Implement this function.
  (void) args;
  hk_state_push_userdata(state, (HkUserdata *) handler_wrapper_new(0));
}

static void close_call(HkState *state, HkValue *args)
{
  // TODO: Implement this function.
  (void) args;
  hk_state_push_nil(state);
}

static void add_call(HkState *state, HkValue *args)
{
  // TODO: Implement this function.
  (void) args;
  hk_state_push_nil(state);
}

static void modify_call(HkState *state, HkValue *args)
{
  // TODO: Implement this function.
  (void) args;
  hk_state_push_nil(state);
}

static void delete_call(HkState *state, HkValue *args)
{
  // TODO: Implement this function.
  (void) args;
  hk_state_push_nil(state);
}

static void wait_call(HkState *state, HkValue *args)
{
  // TODO: Implement this function.
  (void) args;
  hk_state_push_nil(state);
}

HK_LOAD_MODULE_HANDLER(polling)
{
  hk_state_push_string_from_chars(state, -1, "polling");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "new");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "new", 0, new_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "close");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "close", 1, close_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "add");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "add", 3, add_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "modify");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "modify", 3, modify_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "delete");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "delete", 2, delete_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "wait");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "wait", 6, wait_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 6);
}
