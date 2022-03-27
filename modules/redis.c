//
// Hook Programming Language
// redis.c
//

#include "redis.h"
#include <stdlib.h>
#include <hiredis/hiredis.h>

typedef struct
{
  HK_USERDATA_HEADER
  redisContext *ctx;
} redis_context_t;

static inline redis_context_t *redis_context_new(redisContext *ctx);
static void redis_context_deinit(hk_userdata_t *udata);
static hk_value_t redis_reply_to_value(redisReply *reply);
static int32_t connect_call(hk_vm_t *vm, hk_value_t *args);
static int32_t command_call(hk_vm_t *vm, hk_value_t *args);

static inline redis_context_t *redis_context_new(redisContext *ctx)
{
  redis_context_t *redis_context = (redis_context_t *) hk_allocate(sizeof(*redis_context));
  hk_userdata_init((hk_userdata_t *) redis_context, &redis_context_deinit);
  redis_context->ctx = ctx;
  return redis_context;
}

static void redis_context_deinit(hk_userdata_t *udata)
{
  redisFree(((redis_context_t *) udata)->ctx);
}

static hk_value_t redis_reply_to_value(redisReply *reply)
{
  hk_value_t result = HK_NIL_VALUE;
  switch (reply->type)
  {
    case REDIS_REPLY_STRING:
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_ERROR:
      result = hk_string_value(hk_string_from_chars(reply->len, reply->str));
      break;
    case REDIS_REPLY_ARRAY:
      {
        int32_t length = reply->elements;
        hk_array_t *arr = hk_array_new_with_capacity(length);
        arr->length = length;
        for (int32_t i = 0; i < length; ++i)
        {
          redisReply *nested = reply->element[i];
          hk_value_t elem = redis_reply_to_value(nested);
          freeReplyObject(nested);
          hk_value_incr_ref(elem);
          arr->elements[i] = elem;
        }
        result = hk_array_value(arr);
      }
      break;
    case REDIS_REPLY_INTEGER:
      result = hk_float_value((double) reply->integer);
      break;
    case REDIS_REPLY_NIL:
      break;
    default:
      result = hk_string_value(hk_string_from_chars(-1, "unknown reply type"));
      break;
  }
  return result;
}

static int32_t connect_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_int(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  hk_string_t *hostname = hk_as_string(args[1]);
  int32_t port = (int32_t) args[2].as_float;
  redisContext *ctx = redisConnect(hostname->chars, port);
  if (!ctx || ctx->err)
    return hk_vm_push_nil(vm);
  return hk_vm_push_userdata(vm, (hk_userdata_t *) redis_context_new(ctx));
}

static int32_t command_call(hk_vm_t *vm, hk_value_t *args)
{
  if (hk_vm_check_userdata(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_check_string(args, 2) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  redisContext *ctx = ((redis_context_t *) hk_as_userdata(args[1]))->ctx;
  hk_string_t *command = hk_as_string(args[2]);
  redisReply *reply = redisCommand(ctx, command->chars);
  hk_assert(reply, "redisCommand returned NULL");
  hk_value_t result = redis_reply_to_value(reply);
  freeReplyObject(reply);
  return hk_vm_push(vm, result);
}

#ifdef _WIN32
int32_t __declspec(dllexport) __stdcall load_redis(hk_vm_t *vm)
#else
int32_t load_redis(hk_vm_t *vm)
#endif
{
  if (hk_vm_push_string_from_chars(vm, -1, "redis") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "connect") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "connect", 2, &connect_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_string_from_chars(vm, -1, "command") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_vm_push_new_native(vm, "command", 2, &command_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_vm_construct(vm, 2);
}
