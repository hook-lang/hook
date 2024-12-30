//
// json.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "json.h"
#include <stdlib.h>
#include "deps/cJSON.h"

static inline cJSON *value_to_json(HkValue val);
static inline HkValue json_to_value(HkVM *vm, cJSON *json);
static void encode_call(HkVM *vm, HkValue *args);
static void decode_call(HkVM *vm, HkValue *args);

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
        cJSON *json_val = value_to_json(inst->values[i]);
        hk_assert(cJSON_AddItemToObject(json, field.name->chars, json_val),
          "Failed to add item to object.");
      }
    }
    break;
  }
  hk_assert(json, "json is NULL");
  return json;
}

static inline HkValue json_to_value(HkVM *vm, cJSON *json)
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
        HkValue elem = json_to_value(vm, json_elem);
        hk_array_inplace_add_element(arr, elem);
        json_elem = json_elem->next;
      }
      val = hk_array_value(arr);
    }
    break;
  case cJSON_Object:
    {
      hk_vm_push_nil(vm);
      cJSON *json_field = json->child;
      int length = 0;
      while (json_field)
      {
        HkString *field_name = hk_string_from_chars(-1, json_field->string);
        HkValue value = json_to_value(vm, json_field);
        hk_vm_push_string(vm, field_name);
        hk_vm_push(vm, value);
        ++length;
        json_field = json_field->next;
      }
      hk_vm_construct(vm, length);
      HkInstance *inst = hk_as_instance(vm->stackSlots[vm->stackTop]);
      hk_incr_ref(inst);
      hk_vm_pop(vm);
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

static void encode_call(HkVM *vm, HkValue *args)
{
  HkValue val = args[1];
  cJSON *json = value_to_json(val);
  char *chars = cJSON_Print(json);
  cJSON_Delete(json);
  HkString *str = hk_string_from_chars(-1, chars);
  hk_free(chars);
  hk_vm_push_string(vm, str);
  if (!hk_vm_is_ok(vm))
    hk_string_free(str);
}

static void decode_call(HkVM *vm, HkValue *args)
{
  hk_vm_check_argument_string(vm, args, 1);
  hk_return_if_not_ok(vm);
  HkString *str = hk_as_string(args[1]);
  cJSON *json = cJSON_ParseWithLength(str->chars, str->length);
  if (!json)
  {
    hk_vm_runtime_error(vm, "cannot parse json");
    return;
  }
  HkValue val = json_to_value(vm, json);
  cJSON_Delete(json);
  hk_vm_push(vm, val);
  if (!hk_vm_is_ok(vm))
    hk_value_free(val);
}

HK_LOAD_MODULE_HANDLER(json)
{
  hk_vm_push_string_from_chars(vm, -1, "json");
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "encode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "encode", 1, encode_call);
  hk_return_if_not_ok(vm);
  hk_vm_push_string_from_chars(vm, -1, "decode");
  hk_return_if_not_ok(vm);
  hk_vm_push_new_native(vm, "decode", 1, decode_call);
  hk_return_if_not_ok(vm);
  hk_vm_construct(vm, 2);
}
