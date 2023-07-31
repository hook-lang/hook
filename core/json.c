//
// The Hook Programming Language
// json.c
//

#include "json.h"
#include <stdlib.h>
#include <hook/check.h>
#include <hook/status.h>
#include <hook/error.h>
#include "deps/cJSON.h"

static inline cJSON *value_to_json(HkValue val);
static inline HkValue json_to_value(HkState *state, cJSON *json);
static int encode_call(HkState *state, HkValue *args);
static int decode_call(HkState *state, HkValue *args);

static inline cJSON *value_to_json(HkValue val)
{
  cJSON *json = NULL;
  switch (val.type)
  {
  case HK_TYPE_NIL:
    json = cJSON_CreateNull();
    break;
  case HK_TYPE_BOOL:
    json = cJSON_CreateBool((cJSON_bool) hk_as_bool(val));
    break;
  case HK_TYPE_NUMBER:
    json = cJSON_CreateNumber(hk_as_number(val));
    break;
  case HK_TYPE_STRING:
    json = cJSON_CreateString(hk_as_string(val)->chars);
    break;
  case HK_TYPE_RANGE:
  case HK_TYPE_STRUCT:
  case HK_TYPE_ITERATOR:
  case HK_TYPE_CALLABLE:
  case HK_TYPE_USERDATA:
    json = cJSON_CreateNull();
    break;
  case HK_TYPE_ARRAY:
    {
      HkArray *arr = hk_as_array(val);
      json = cJSON_CreateArray();
      for (int i = 0; i < arr->length; ++i)
      {
        HkValue elem = arr->elements[i];
        cJSON *json_elem = value_to_json(elem);
        hk_assert(cJSON_AddItemToArray(json, json_elem), "Failed to add item to array.");
      }
    }
    break;
  case HK_TYPE_INSTANCE:
    {
      HkInstance *inst = hk_as_instance(val);
      json = cJSON_CreateObject();
      HkStruct *ztruct = inst->ztruct;
      HkField *fields = ztruct->fields;
      for (int i = 0; i < ztruct->length; ++i)
      {
        HkField field = fields[i];
        HkValue val = inst->values[i];
        cJSON *json_val = value_to_json(val);
        hk_assert(cJSON_AddItemToObject(json, field.name->chars, json_val), "Failed to add item to object.");
      }
    }
    break;
  }
  hk_assert(json, "json is NULL");
  return json;
}

static inline HkValue json_to_value(HkState *state, cJSON *json)
{
  HkValue val;
  switch (json->type)
  {
  case cJSON_False:    
    val = HK_FALSE_VALUE;
    break;
  case cJSON_True:
    val = HK_TRUE_VALUE;
    break;
  case cJSON_NULL:
    val = HK_NIL_VALUE;
    break;
  case cJSON_Number:
    val = hk_number_value(json->valuedouble);
    break;
  case cJSON_String:
  case cJSON_Raw:
    val = hk_string_value(hk_string_from_chars(-1, json->valuestring));
    break;
  case cJSON_Array:
    {
      HkArray *arr = hk_array_new();
      cJSON *json_elem = json->child;
      while (json_elem)
      {
        HkValue elem = json_to_value(state, json_elem);
        hk_array_inplace_add_element(arr, elem);
        json_elem = json_elem->next;
      }
      val = hk_array_value(arr);
    }
    break;
  case cJSON_Object:
    {
      hk_state_push_nil(state);
      cJSON *json_field = json->child;
      int length = 0;
      while (json_field)
      {
        HkString *field_name = hk_string_from_chars(-1, json_field->string);
        HkValue value = json_to_value(state, json_field);
        hk_state_push_string(state, field_name);
        hk_state_push(state, value);
        ++length;
        json_field = json_field->next;
      }
      hk_state_construct(state, length);
      HkInstance *inst = hk_as_instance(state->stackSlots[state->stackTop]);
      hk_incr_ref(inst);
      hk_state_pop(state);
      hk_decr_ref(inst);
      val = hk_instance_value(inst);
    }
    break;
  default:
    val = HK_NIL_VALUE;
    break;
  }
  return val;
}

static int encode_call(HkState *state, HkValue *args)
{
  HkValue val = args[1];
  cJSON *json = value_to_json(val);
  char *chars = cJSON_Print(json);
  cJSON_Delete(json);
  HkString *str = hk_string_from_chars(-1, chars);
  free(chars);
  if (hk_state_push_string(state, str) == HK_STATUS_ERROR)
  {
    hk_string_free(str);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

static int decode_call(HkState *state, HkValue *args)
{
  if (hk_check_argument_string(args, 1) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  HkString *str = hk_as_string(args[1]);
  cJSON *json = cJSON_ParseWithLength(str->chars, str->length);
  if (!json)
  {
    hk_runtime_error("cannot parse json");
    return HK_STATUS_ERROR;
  }
  HkValue val = json_to_value(state, json);
  cJSON_Delete(json);
  if (hk_state_push(state, val) == HK_STATUS_ERROR)
  {
    hk_value_free(val);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

HK_LOAD_FN(json)
{
  if (hk_state_push_string_from_chars(state, -1, "json") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "encode") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "encode", 1, &encode_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_string_from_chars(state, -1, "decode") == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  if (hk_state_push_new_native(state, "decode", 1, &decode_call) == HK_STATUS_ERROR)
    return HK_STATUS_ERROR;
  return hk_state_construct(state, 2);
}
