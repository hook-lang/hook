//
// Hook Programming Language
// value.c
//

#include "value.h"
#include <stdlib.h>
#include "range.h"
#include "struct.h"
#include "callable.h"
#include "userdata.h"
#include "common.h"
#include "error.h"

static inline void value_free(value_t val);

static inline void value_free(value_t val)
{
  switch (val.type)
  {
  case TYPE_NIL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
    break;
  case TYPE_STRING:
    string_free(AS_STRING(val));
    break;
  case TYPE_RANGE:
    range_free(AS_RANGE(val));
    break;
  case TYPE_ARRAY:
    array_free(AS_ARRAY(val));
    break;
  case TYPE_STRUCT:
    struct_free(AS_STRUCT(val));
    break;
  case TYPE_INSTANCE:
    instance_free(AS_INSTANCE(val));
    break;
  case TYPE_ITERATOR:
    iterator_free(AS_ITERATOR(val));
    break;
  case TYPE_CALLABLE:
    {
      if (IS_NATIVE(val))
      {
        native_free(AS_NATIVE(val));
        break;
      }
      closure_free(AS_CLOSURE(val));
    }
    break;
  case TYPE_USERDATA:
    userdata_free(AS_USERDATA(val));
    break;
  }
}

const char *type_name(type_t type)
{
  char *name = "nil";
  switch (type)
  {
  case TYPE_NIL:
    break;
  case TYPE_BOOLEAN:
    name = "boolean";
    break;
  case TYPE_NUMBER:
    name = "number";
    break;
  case TYPE_STRING:
    name = "string";
    break;
  case TYPE_RANGE:
    name = "range";
    break;
  case TYPE_ARRAY:
    name = "array";
    break;
  case TYPE_STRUCT:
    name = "struct";
    break;
  case TYPE_INSTANCE:
    name = "instance";
    break;
  case TYPE_ITERATOR:
    name = "iterator";
    break;
  case TYPE_CALLABLE:
    name = "callable";
    break;
  case TYPE_USERDATA:
    name = "userdata";
    break;
  }
  return name;
}

void value_release(value_t val)
{
  if (!IS_OBJECT(val))
    return;
  object_t *obj = AS_OBJECT(val);
  DECR_REF(obj);
  if (IS_UNREACHABLE(obj))
    value_free(val);
}

void value_print(value_t val, bool quoted)
{
  switch (val.type)
  {
  case TYPE_NIL:
    printf("nil");
    break;
  case TYPE_BOOLEAN:
    printf("%s", val.as.boolean ? "true" : "false");
    break;
  case TYPE_NUMBER:
    printf("%g", val.as.number);
    break;
  case TYPE_STRING:
    string_print(AS_STRING(val), quoted);
    break;
  case TYPE_RANGE:
    range_print(AS_RANGE(val));
    break;
  case TYPE_ARRAY:
    array_print(AS_ARRAY(val));
    break;
  case TYPE_STRUCT:
    {
      string_t *name = AS_STRUCT(val)->name;
      if (name)
      {
        printf("<struct %.*s at %p>", name->length, name->chars, val.as.pointer);
        break;
      }
      printf("<struct at %p>", val.as.pointer);
    }
    break;
  case TYPE_INSTANCE:
    instance_print(AS_INSTANCE(val));
    break;
  case TYPE_ITERATOR:
    printf("<iterator at %p>", val.as.pointer);
    break;
  case TYPE_CALLABLE:
    {
      string_t *name = IS_NATIVE(val) ? AS_NATIVE(val)->name : AS_CLOSURE(val)->fn->name;
      if (name)
      {
        printf("<callable %.*s at %p>", name->length, name->chars, val.as.pointer);
        break;
      }
      printf("<callable at %p>", val.as.pointer);
    }
    break;
  case TYPE_USERDATA:
    printf("<userdata at %p>", val.as.pointer);
    break;
  }
}

bool value_equal(value_t val1, value_t val2)
{
  if (val1.type != val2.type)
    return false;
  bool result = true;
  switch (val1.type)
  {
  case TYPE_NIL:
    break;
  case TYPE_BOOLEAN:
    result = val1.as.boolean == val2.as.boolean;
    break;
  case TYPE_NUMBER:
    result = val1.as.number == val2.as.number;
    break;
  case TYPE_STRING:
    result = string_equal(AS_STRING(val1), AS_STRING(val2));
    break;
  case TYPE_RANGE:
    result = range_equal(AS_RANGE(val1), AS_RANGE(val2));
    break;
  case TYPE_ARRAY:
    result = array_equal(AS_ARRAY(val1), AS_ARRAY(val2));
    break;
  case TYPE_STRUCT:
    result = struct_equal(AS_STRUCT(val1), AS_STRUCT(val2));
    break;
  case TYPE_INSTANCE:
    instance_equal(AS_INSTANCE(val1), AS_INSTANCE(val2));
    break;
  case TYPE_ITERATOR:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    result = val1.as.pointer == val2.as.pointer;
    break;
  }
  return result;
}

int value_compare(value_t val1, value_t val2, int *result)
{
  if (val1.type != val2.type)
  {
    runtime_error("cannot compare `%s` and `%s`", type_name(val1.type),
      type_name(val2.type));
    return STATUS_ERROR;
  }
  switch (val1.type)
  {
  case TYPE_NIL:
    *result = 0;
    return STATUS_OK;
  case TYPE_BOOLEAN:
    *result = val1.as.boolean - val2.as.boolean;
    return STATUS_OK;
  case TYPE_NUMBER:
    if (val1.as.number > val2.as.number)
    {
      *result = 1;
      return STATUS_OK;
    }
    if (val1.as.number < val2.as.number)
    {
      *result = -1;
      return STATUS_OK;
    }
    *result = 0;
    return STATUS_OK;
  case TYPE_STRING:
    *result = string_compare(AS_STRING(val1), AS_STRING(val2));
    return STATUS_OK;
  case TYPE_RANGE:
    *result = range_compare(AS_RANGE(val1), AS_RANGE(val2));
    return STATUS_OK;
  case TYPE_ARRAY:
    return array_compare(AS_ARRAY(val1), AS_ARRAY(val2), result);
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_ITERATOR:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    break;
  }
  runtime_error("cannot compare value of type `%s`", type_name(val1.type));
  return STATUS_ERROR;
}

void value_serialize(value_t val, FILE *stream)
{
  int type = val.type;
  int flags = val.flags;
  fwrite(&type, sizeof(type), 1, stream);
  fwrite(&flags, sizeof(flags), 1, stream);
  switch ((type_t) type)
  {
  case TYPE_NUMBER:
    fwrite(&val.as.number, sizeof(val.as.number), 1, stream);
    break;
  case TYPE_STRING:
    string_serialize(AS_STRING(val), stream);
    break;
  case TYPE_NIL:
  case TYPE_BOOLEAN:
  case TYPE_RANGE:
  case TYPE_ARRAY:
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_ITERATOR:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    ASSERT(false, "unimplemented serialization");
    break;
  }
}

bool value_deserialize(FILE *stream, value_t *result)
{
  int type;
  int flags;
  if (fread(&type, sizeof(type), 1, stream) != 1)
    return false;
  if (fread(&flags, sizeof(flags), 1, stream) != 1)
    return false;
  value_t val;
  switch ((type_t) type)
  {
  case TYPE_NUMBER:
    {
      double data;
      if (fread(&data, sizeof(data), 1, stream) != 1)
        return false;
      val = NUMBER_VALUE(data);
    }
    break;
  case TYPE_STRING:
    {
      string_t *str = string_deserialize(stream);
      if (!str)
        return false;
      val = STRING_VALUE(str);
    }
    break;
  case TYPE_NIL:
  case TYPE_BOOLEAN:
  case TYPE_RANGE:
  case TYPE_ARRAY:
  case TYPE_STRUCT:
  case TYPE_INSTANCE:
  case TYPE_ITERATOR:
  case TYPE_CALLABLE:
  case TYPE_USERDATA:
    ASSERT(false, "unimplemented deserialization");
    break;
  }
  *result = val;
  return true;
}
