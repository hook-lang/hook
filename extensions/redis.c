//
// The Hook Programming Language
// redis.c
//

#include "redis.h"
#include <stdlib.h>
#include <hiredis/hiredis.h>
#include <hook/memory.h>
#include <hook/check.h>
#include <hook/status.h>

typedef struct
{
  HK_USERDATA_HEADER
  redisContext *redis_context;
} RedisContextWrapper;

static inline RedisContextWrapper *redis_context_wrapper_new(redisContext *redis_context);
static void redis_context_wrapper_deinit(HkUserdata *udata);
static HkValue redis_reply_to_value(redisReply *reply);
static int connect_call(HkState *state, HkValue *args);
static int command_call(HkState *state, HkValue *args);

static inline RedisContextWrapper *redis_context_wrapper_new(redisContext *redis_context)
{
  RedisContextWrapper *wrapper = (RedisContextWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, &redis_context_wrapper_deinit);
  wrapper->redis_context = redis_context;
  return wrapper;
}

static void redis_context_wrapper_deinit(HkUserdata *udata)
{
  redisFree(((RedisContextWrapper *) udata)->redis_context);
}

static HkValue redis_reply_to_value(redisReply *reply)
{
  HkValue result = HK_NIL_VALUE;
  switch (reply->type)
  {
    case REDIS_REPLY_STRING:
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_ERROR:
      result = hk_string_value(hk_string_from_chars(reply->len, reply->str));
      break;
    case REDIS_REPLY_ARRAY:
    case REDIS_REPLY_SET:
      {
        int length = (int) reply->elements;
        HkArray *arr = hk_array_new_with_capacity(length);
        arr->length = length;
        for (int i = 0; i < length; ++i)
        {
          redisReply *nested = reply->element[i];
          HkValue elem = redis_reply_to_value(nested);
          freeReplyObject(nested);
          hk_value_incr_ref(elem);
          arr->elements[i] = elem;
        }
        result = hk_array_value(arr);
      }
      break;
    case REDIS_REPLY_INTEGER:
      result = hk_number_value((double) reply->integer);
      break;
    case REDIS_REPLY_NIL:
      result = HK_NIL_VALUE;
      break;
    case REDIS_REPLY_DOUBLE:
      {
        double data;
        (void) hk_double_from_chars(&data, reply->str, false);
        result = hk_number_value(data);
      }
      break;
    case REDIS_REPLY_BOOL:
      result = reply->integer ? HK_TRUE_VALUE : HK_FALSE_VALUE;
      break;
    case REDIS_REPLY_MAP:
    case REDIS_REPLY_ATTR:
    case REDIS_REPLY_PUSH:
    case REDIS_REPLY_BIGNUM:
    case REDIS_REPLY_VERB:
      result = hk_string_value(hk_string_from_chars(-1, "unsupported reply type"));
      break;
    default:
      result = hk_string_value(hk_string_from_chars(-1, "unknown reply type"));
      break;
  }
  return result;
}

static int connect_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *hostname = hk_as_string(args[1]);
  int port = (int) hk_as_number(args[2]);
  redisContext *redis_context = redisConnect(hostname->chars, port);
  if (!redis_context || redis_context->err)
    return hk_state_push_nil(state);
  return hk_state_push_userdata(state, (HkUserdata *) redis_context_wrapper_new(redis_context));
}

static int command_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_check_argument_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  redisContext *redis_context = ((RedisContextWrapper *) hk_as_userdata(args[1]))->redis_context;
  HkString *command = hk_as_string(args[2]);
  redisReply *reply = redisCommand(redis_context, command->chars);
  hk_assert(reply, "redisCommand returned NULL");
  HkValue result = redis_reply_to_value(reply);
  freeReplyObject(reply);
  return hk_state_push(state, result);
}

HK_LOAD_FN(redis)
{
  if (hk_state_push_string_from_chars(state, -1, "redis") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "connect") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "connect", 2, &connect_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "command") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "command", 2, &command_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 2);
}
