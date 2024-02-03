//
// The Hook Programming Language
// log.c
//

#include "log.h"
#include <time.h>

static inline void get_local_time(char *buf);
static inline void log(const char *level, HkClosure *cl, HkString *msg);
static void trace_call(HkState *state, HkValue *args);
static void debug_call(HkState *state, HkValue *args);
static void info_call(HkState *state, HkValue *args);
static void warn_call(HkState *state, HkValue *args);
static void error_call(HkState *state, HkValue *args);
static void fatal_call(HkState *state, HkValue *args);

static inline void get_local_time(char *buf)
{
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  int len = strftime(buf, 64, "%Y-%m-%d %H:%M:%S", tm);
  buf[len] = '\0';
}

static inline void log(const char *level, HkClosure *cl, HkString *msg)
{
  // TODO: Add file and line number.
  (void) cl;
  char buf[64];
  get_local_time(buf);
  fprintf(stdout, "%s %-5s %.*s\n", buf, level, msg->length, msg->chars);
  fflush(stdout);
}

static void trace_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkClosure *cl = hk_as_closure(args[0]);
  HkString *msg = hk_as_string(args[1]);
  log("TRACE", cl, msg);
  hk_state_push_nil(state);
}

static void debug_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkClosure *cl = hk_as_closure(args[0]);
  HkString *msg = hk_as_string(args[1]);
  log("DEBUG", cl, msg);
  hk_state_push_nil(state);
}

static void info_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkClosure *cl = hk_as_closure(args[0]);
  HkString *msg = hk_as_string(args[1]);
  log("INFO", cl, msg);
  hk_state_push_nil(state);
}

static void warn_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkClosure *cl = hk_as_closure(args[0]);
  HkString *msg = hk_as_string(args[1]);
  log("WARN", cl, msg);
  hk_state_push_nil(state);
}

static void error_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkClosure *cl = hk_as_closure(args[0]);
  HkString *msg = hk_as_string(args[1]);
  log("ERROR", cl, msg);
  hk_state_push_nil(state);
}

static void fatal_call(HkState *state, HkValue *args)
{
  hk_state_check_argument_string(state, args, 1);
  hk_return_if_not_ok(state);
  HkClosure *cl = hk_as_closure(args[0]);
  HkString *msg = hk_as_string(args[1]);
  log("FATAL", cl, msg);
  hk_state_push_nil(state);
}

HK_LOAD_MODULE_HANDLER(log)
{
  hk_state_push_string_from_chars(state, -1, "log");
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "trace");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "trace", 1, trace_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "debug");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "debug", 1, debug_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "info");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "info", 1, info_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "warn");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "warn", 1, warn_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "error");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "error", 1, error_call);
  hk_return_if_not_ok(state);
  hk_state_push_string_from_chars(state, -1, "fatal");
  hk_return_if_not_ok(state);
  hk_state_push_new_native(state, "fatal", 1, fatal_call);
  hk_return_if_not_ok(state);
  hk_state_construct(state, 6);
}
