//
// redis.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "redis.h"
#include <stdlib.h>
#include <hiredis/hiredis.h>

typedef struct
{
  HK_USERDATA_HEADER
  redisContext *context;
} RedisContextWrapper;

static inline RedisContextWrapper *redis_context_wrapper_new(redisContext *context);
static void redis_context_wrapper_deinit(HkUserdata *udata);
static HkValue redis_reply_to_value(redisReply *reply);
static void connect_call(HkVM *vm, HkValue *args);
static void command_call(HkVM *vm, HkValue *args);

static inline RedisContextWrapper *redis_context_wrapper_new(redisContext *context)
{
  RedisContextWrapper *wrapper = (RedisContextWrapper *) hk_allocate(sizeof(*wrapper));
  hk_userdata_init((HkUserdata *) wrapper, redis_context_wrapper_deinit);
  wrapper->context = context;
  return wrapper;
}

static void redis_context_wrapper_deinit(HkUserdata *udata)
{
  redisFree(((RedisContextWrapper *) udata)->context);
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

static void connect_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_int(vm, args, 2);
  hk_return_if_not_ok(vm);
  HkString *hostname = hk_as_string(args[1]);
  int port = (int) hk_as_number(args[2]);
  redisContext *context = redisConnect(hostname->chars, port);
  if (!context || context->err)
  {
    hk_vm_push_nil(vm);
    return;
  }
  hk_vm_push_userdata(vm, (HkUserdata *) redis_context_wrapper_new(context));
}

static void command_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_userdata(vm, args, 1);
  hk_return_if_not_ok(vm);
  hk_vm_check_argument_string(vm, args, 2);
  hk_return_if_not_ok(vm);
  redisContext *context = ((RedisContextWrapper *) hk_as_userdata(args[1]))->context;
  HkString *command = hk_as_string(args[2]);
  redisReply *reply = redisCommand(context, command->chars);
  hk_assert(reply, "redisCommand returned NULL");
  HkValue result = redis_reply_to_value(reply);
  freeReplyObject(reply);
  hk_vm_push(vm, result);
}

HK_LOAD_MODULE_HANDLER(redis)
{
  hk_vm_push_string_from_chars(vm, -1, "redis");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "connect");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "connect", 2, connect_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "command");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "command", 2, command_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 2);
}
