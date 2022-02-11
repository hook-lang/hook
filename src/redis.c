//
// Hook Programming Language
// redis.c
//

#include "redis.h"
#include <stdlib.h>
#include <hiredis/hiredis.h>
#include "common.h"
#include "memory.h"

typedef struct
{
  USERDATA_HEADER
  redisContext *ctx;
} redis_context_t;

static inline redis_context_t *redis_context_new(redisContext *ctx);
static void redis_context_deinit(userdata_t *udata);
static value_t redis_reply_to_value(redisReply *reply);
static int connect_call(vm_t *vm, value_t *args);
static int command_call(vm_t *vm, value_t *args);

static inline redis_context_t *redis_context_new(redisContext *ctx)
{
  redis_context_t *redis_context = (redis_context_t *) allocate(sizeof(*redis_context));
  userdata_init((userdata_t *) redis_context, &redis_context_deinit);
  redis_context->ctx = ctx;
  return redis_context;
}

static void redis_context_deinit(userdata_t *udata)
{
  redisFree(((redis_context_t *) udata)->ctx);
}

static value_t redis_reply_to_value(redisReply *reply)
{
  value_t result = NIL_VALUE;
  switch (reply->type)
  {
    case REDIS_REPLY_STRING:
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_ERROR:
      result = STRING_VALUE(string_from_chars(reply->len, reply->str));
      break;
    case REDIS_REPLY_ARRAY:
      {
        int length = reply->elements;
        array_t *arr = array_allocate(length);
        arr->length = length;
        for (int i = 0; i < length; ++i)
        {
          redisReply *nested = reply->element[i];
          value_t elem = redis_reply_to_value(nested);
          freeReplyObject(nested);
          VALUE_INCR_REF(elem);
          arr->elements[i] = elem;
        }
        result = ARRAY_VALUE(arr);
      }
      break;
    case REDIS_REPLY_INTEGER:
      result = NUMBER_VALUE((double) reply->integer);
      break;
    case REDIS_REPLY_NIL:
      break;
    default:
      result = STRING_VALUE(string_from_chars(-1, "unknown reply type"));
      break;
  }
  return result;
}

static int connect_call(vm_t *vm, value_t *args)
{
  if (vm_check_userdata(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_int(args, 2) == STATUS_ERROR)
    return STATUS_ERROR;
  string_t *hostname = AS_STRING(args[1]);
  int port = (int) args[2].as.number;
  redisContext *ctx = redisConnect(hostname->chars, port);
  if (!ctx || ctx->err)
    return vm_push_nil(vm);
  return vm_push_userdata(vm, (userdata_t *) redis_context_new(ctx));
}

static int command_call(vm_t *vm, value_t *args)
{
  if (vm_check_userdata(args, 1) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_check_string(args, 2) == STATUS_ERROR)
    return STATUS_ERROR;
  redisContext *ctx = ((redis_context_t *) AS_USERDATA(args[1]))->ctx;
  string_t *command = AS_STRING(args[2]);
  redisReply *reply = redisCommand(ctx, command->chars);
  ASSERT(reply, "redisCommand returned NULL");
  value_t result = redis_reply_to_value(reply);
  freeReplyObject(reply);
  return vm_push(vm, result);
}

#ifdef _WIN32
int __declspec(dllexport) __stdcall load_redis(vm_t *vm)
#else
int load_redis(vm_t *vm)
#endif
{
  if (vm_push_string_from_chars(vm, -1, "redis") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "connect") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "connect", 2, &connect_call) == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_string_from_chars(vm, -1, "command") == STATUS_ERROR)
    return STATUS_ERROR;
  if (vm_push_new_native(vm, "command", 2, &command_call) == STATUS_ERROR)
    return STATUS_ERROR;
  return vm_construct(vm, 2);
}
