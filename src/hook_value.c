//
// Hook Programming Language
// hook_value.c
//

#include "hook_value.h"
#include <stdlib.h>
#include "hook_range.h"
#include "hook_struct.h"
#include "hook_callable.h"
#include "hook_userdata.h"
#include "hook_utils.h"
#include "hook_status.h"
#include "hook_error.h"

static inline void value_free(hk_value_t val);

static inline void value_free(hk_value_t val)
{
  switch (val.type)
  {
  case HK_TYPE_NIL:
  case HK_TYPE_BOOL:
  case HK_TYPE_FLOAT:
    break;
  case HK_TYPE_STRING:
    hk_string_free(hk_as_string(val));
    break;
  case HK_TYPE_RANGE:
    hk_range_free(hk_as_range(val));
    break;
  case HK_TYPE_ARRAY:
    hk_array_free(hk_as_array(val));
    break;
  case HK_TYPE_STRUCT:
    hk_struct_free(hk_as_struct(val));
    break;
  case HK_TYPE_INSTANCE:
    hk_instance_free(hk_as_instance(val));
    break;
  case HK_TYPE_ITERATOR:
    hk_iterator_free(hk_as_iterator(val));
    break;
  case HK_TYPE_CALLABLE:
    {
      if (hk_is_native(val))
      {
        hk_native_free(hk_as_native(val));
        break;
      }
      hk_closure_free(hk_as_closure(val));
    }
    break;
  case HK_TYPE_USERDATA:
    hk_userdata_free(hk_as_userdata(val));
    break;
  }
}

const char *hk_type_name(int32_t type)
{
  char *name = "nil";
  switch (type)
  {
  case HK_TYPE_NIL:
    break;
  case HK_TYPE_BOOL:
    name = "bool";
    break;
  case HK_TYPE_FLOAT:
    name = "float";
    break;
  case HK_TYPE_STRING:
    name = "string";
    break;
  case HK_TYPE_RANGE:
    name = "range";
    break;
  case HK_TYPE_ARRAY:
    name = "array";
    break;
  case HK_TYPE_STRUCT:
    name = "struct";
    break;
  case HK_TYPE_INSTANCE:
    name = "instance";
    break;
  case HK_TYPE_ITERATOR:
    name = "iterator";
    break;
  case HK_TYPE_CALLABLE:
    name = "callable";
    break;
  case HK_TYPE_USERDATA:
    name = "userdata";
    break;
  }
  return name;
}

void hk_value_release(hk_value_t val)
{
  if (!hk_is_object(val))
    return;
  hk_object_t *obj = hk_as_object(val);
  hk_decr_ref(obj);
  if (hk_is_unreachable(obj))
    value_free(val);
}

void hk_value_print(hk_value_t val, bool quoted)
{
  switch (val.type)
  {
  case HK_TYPE_NIL:
    printf("nil");
    break;
  case HK_TYPE_BOOL:
    printf("%s", val.as_bool ? "true" : "false");
    break;
  case HK_TYPE_FLOAT:
    printf("%g", val.as_float);
    break;
  case HK_TYPE_STRING:
    hk_string_print(hk_as_string(val), quoted);
    break;
  case HK_TYPE_RANGE:
    hk_range_print(hk_as_range(val));
    break;
  case HK_TYPE_ARRAY:
    hk_array_print(hk_as_array(val));
    break;
  case HK_TYPE_STRUCT:
    {
      hk_string_t *name = hk_as_struct(val)->name;
      if (name)
      {
        printf("<struct %.*s at %p>", name->length, name->chars, val.as_pointer);
        break;
      }
      printf("<struct at %p>", val.as_pointer);
    }
    break;
  case HK_TYPE_INSTANCE:
    hk_instance_print(hk_as_instance(val));
    break;
  case HK_TYPE_ITERATOR:
    printf("<iterator at %p>", val.as_pointer);
    break;
  case HK_TYPE_CALLABLE:
    {
      hk_string_t *name = hk_is_native(val) ? hk_as_native(val)->name : hk_as_closure(val)->fn->name;
      if (name)
      {
        printf("<callable %.*s at %p>", name->length, name->chars, val.as_pointer);
        break;
      }
      printf("<callable at %p>", val.as_pointer);
    }
    break;
  case HK_TYPE_USERDATA:
    printf("<userdata at %p>", val.as_pointer);
    break;
  }
}

bool hk_value_equal(hk_value_t val1, hk_value_t val2)
{
  if (val1.type != val2.type)
    return false;
  bool result = true;
  switch (val1.type)
  {
  case HK_TYPE_NIL:
    break;
  case HK_TYPE_BOOL:
    result = val1.as_bool == val2.as_bool;
    break;
  case HK_TYPE_FLOAT:
    result = val1.as_float == val2.as_float;
    break;
  case HK_TYPE_STRING:
    result = hk_string_equal(hk_as_string(val1), hk_as_string(val2));
    break;
  case HK_TYPE_RANGE:
    result = hk_range_equal(hk_as_range(val1), hk_as_range(val2));
    break;
  case HK_TYPE_ARRAY:
    result = hk_array_equal(hk_as_array(val1), hk_as_array(val2));
    break;
  case HK_TYPE_STRUCT:
    result = hk_struct_equal(hk_as_struct(val1), hk_as_struct(val2));
    break;
  case HK_TYPE_INSTANCE:
    result = hk_instance_equal(hk_as_instance(val1), hk_as_instance(val2));
    break;
  default:
    result = val1.as_pointer == val2.as_pointer;
    break;
  }
  return result;
}

int32_t hk_value_compare(hk_value_t val1, hk_value_t val2, int32_t *result)
{
  if (val1.type != val2.type)
  {
    hk_runtime_error("type error: cannot compare %s and %s", hk_type_name(val1.type),
      hk_type_name(val2.type));
    return HK_STATUS_ERROR;
  }
  switch (val1.type)
  {
  case HK_TYPE_NIL:
    *result = 0;
    return HK_STATUS_OK;
  case HK_TYPE_BOOL:
    *result = val1.as_bool - val2.as_bool;
    return HK_STATUS_OK;
  case HK_TYPE_FLOAT:
    if (val1.as_float > val2.as_float)
    {
      *result = 1;
      return HK_STATUS_OK;
    }
    if (val1.as_float < val2.as_float)
    {
      *result = -1;
      return HK_STATUS_OK;
    }
    *result = 0;
    return HK_STATUS_OK;
  case HK_TYPE_STRING:
    *result = hk_string_compare(hk_as_string(val1), hk_as_string(val2));
    return HK_STATUS_OK;
  case HK_TYPE_RANGE:
    *result = hk_range_compare(hk_as_range(val1), hk_as_range(val2));
    return HK_STATUS_OK;
  case HK_TYPE_ARRAY:
    return hk_array_compare(hk_as_array(val1), hk_as_array(val2), result);
  default:
    break;
  }
  hk_runtime_error("type error: value of type %s is not comparable", hk_type_name(val1.type));
  return HK_STATUS_ERROR;
}

void hk_value_serialize(hk_value_t val, FILE *stream)
{
  int32_t type = val.type;
  int32_t flags = val.flags;
  fwrite(&type, sizeof(type), 1, stream);
  fwrite(&flags, sizeof(flags), 1, stream);
  if (type == HK_TYPE_FLOAT)
  {
    fwrite(&val.as_float, sizeof(val.as_float), 1, stream);
    return;
  }
  if (type == HK_TYPE_STRING)
  {
    hk_string_serialize(hk_as_string(val), stream);
    return;
  }
  hk_assert(false, "unimplemented serialization");
}

bool hk_value_deserialize(FILE *stream, hk_value_t *result)
{
  int32_t type;
  int32_t flags;
  if (fread(&type, sizeof(type), 1, stream) != 1)
    return false;
  if (fread(&flags, sizeof(flags), 1, stream) != 1)
    return false;
  hk_assert(type == HK_TYPE_FLOAT || type == HK_TYPE_STRING, "unimplemented deserialization");
  if (type == HK_TYPE_FLOAT)
  {
    double data;
    if (fread(&data, sizeof(data), 1, stream) != 1)
      return false;
    *result = hk_float_value(data);
    return true;
  }
  hk_string_t *str = hk_string_deserialize(stream);
  if (!str)
    return false;
  *result = hk_string_value(str);
  return true;
}
