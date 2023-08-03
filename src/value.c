//
// The Hook Programming Language
// value.c
//

#include <hook/value.h>
#include <stdlib.h>
#include <hook/range.h>
#include <hook/struct.h>
#include <hook/callable.h>
#include <hook/userdata.h>
#include <hook/error.h>
#include <hook/utils.h>

void hk_value_free(HkValue val)
{
  switch (val.type)
  {
  case HK_TYPE_NIL:
  case HK_TYPE_BOOL:
  case HK_TYPE_NUMBER:
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

const char *hk_type_name(HkType type)
{
  char *name = "nil";
  switch (type)
  {
  case HK_TYPE_NIL:
    break;
  case HK_TYPE_BOOL:
    name = "bool";
    break;
  case HK_TYPE_NUMBER:
    name = "number";
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

void hk_value_release(HkValue val)
{
  if (!hk_is_object(val))
    return;
  HkObject *obj = hk_as_object(val);
  hk_decr_ref(obj);
  if (hk_is_unreachable(obj))
    hk_value_free(val);
}

void hk_value_print(HkValue val, bool quoted)
{
  switch (val.type)
  {
  case HK_TYPE_NIL:
    printf("nil");
    break;
  case HK_TYPE_BOOL:
    printf("%s", hk_as_bool(val) ? "true" : "false");
    break;
  case HK_TYPE_NUMBER:
    printf("%g", hk_as_number(val));
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
      HkString *name = hk_as_struct(val)->name;
      if (name)
      {
        printf("<struct %.*s at %p>", name->length, name->chars, val.as.pointer);
        break;
      }
      printf("<struct at %p>", val.as.pointer);
    }
    break;
  case HK_TYPE_INSTANCE:
    hk_instance_print(hk_as_instance(val));
    break;
  case HK_TYPE_ITERATOR:
    printf("<iterator at %p>", val.as.pointer);
    break;
  case HK_TYPE_CALLABLE:
    {
      HkString *name = hk_is_native(val) ? hk_as_native(val)->name : hk_as_closure(val)->fn->name;
      if (name)
      {
        printf("<callable %.*s at %p>", name->length, name->chars, val.as.pointer);
        break;
      }
      printf("<callable at %p>", val.as.pointer);
    }
    break;
  case HK_TYPE_USERDATA:
    printf("<userdata at %p>", val.as.pointer);
    break;
  }
}

bool hk_value_equal(HkValue val1, HkValue val2)
{
  if (val1.type != val2.type)
    return false;
  bool result = true;
  switch (val1.type)
  {
  case HK_TYPE_NIL:
    break;
  case HK_TYPE_BOOL:
    result = hk_as_bool(val1) == hk_as_bool(val2);
    break;
  case HK_TYPE_NUMBER:
    result = hk_as_number(val1) == hk_as_number(val2);
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
    result = val1.as.pointer == val2.as.pointer;
    break;
  }
  return result;
}

bool hk_value_compare(HkValue val1, HkValue val2, int *result)
{
  if (val1.type != val2.type)
    return false;
  switch (val1.type)
  {
  case HK_TYPE_NIL:
    *result = 0;
    return true;
  case HK_TYPE_BOOL:
    *result = hk_as_bool(val1) - hk_as_bool(val2);
    return true;
  case HK_TYPE_NUMBER:
    if (hk_as_number(val1) > hk_as_number(val2))
    {
      *result = 1;
      return true;
    }
    if (hk_as_number(val1) < hk_as_number(val2))
    {
      *result = -1;
      return true;
    }
    *result = 0;
    return true;
  case HK_TYPE_STRING:
    *result = hk_string_compare(hk_as_string(val1), hk_as_string(val2));
    return true;
  case HK_TYPE_RANGE:
    *result = hk_range_compare(hk_as_range(val1), hk_as_range(val2));
    return true;
  case HK_TYPE_ARRAY:
    return hk_array_compare(hk_as_array(val1), hk_as_array(val2), result);
  default:
    break;
  }
  return false;
}

void hk_value_serialize(HkValue val, FILE *stream)
{
  HkType type = val.type;
  int flags = val.flags;
  fwrite(&type, sizeof(type), 1, stream);
  fwrite(&flags, sizeof(flags), 1, stream);
  if (type == HK_TYPE_NUMBER)
  {
    fwrite(&val.as.number, sizeof(val.as.number), 1, stream);
    return;
  }
  if (type == HK_TYPE_STRING)
  {
    hk_string_serialize(hk_as_string(val), stream);
    return;
  }
  hk_assert(false, "unimplemented serialization");
}

bool hk_value_deserialize(FILE *stream, HkValue *result)
{
  HkType type;
  int flags;
  if (fread(&type, sizeof(type), 1, stream) != 1)
    return false;
  if (fread(&flags, sizeof(flags), 1, stream) != 1)
    return false;
  hk_assert(type == HK_TYPE_NUMBER || type == HK_TYPE_STRING, "unimplemented deserialization");
  if (type == HK_TYPE_NUMBER)
  {
    double data;
    if (fread(&data, sizeof(data), 1, stream) != 1)
      return false;
    *result = hk_number_value(data);
    return true;
  }
  HkString *str = hk_string_deserialize(stream);
  if (!str)
    return false;
  *result = hk_string_value(str);
  return true;
}
