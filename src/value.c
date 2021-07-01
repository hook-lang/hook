//
// Hook Programming Language
// value.c
//

#include "value.h"
#include "string.h"
#include "array.h"
#include "error.h"

static inline void value_free(value_t val);

static inline void value_free(value_t val)
{
  switch (val.type)
  {
  case TYPE_NULL:
  case TYPE_BOOLEAN:
  case TYPE_NUMBER:
    break;
  case TYPE_STRING:
    string_free(AS_STRING(val));
    break;
  case TYPE_ARRAY:
    array_free(AS_ARRAY(val));
    break;
  }
}

const char *type_name(type_t type)
{
  char *name = "null";
  switch (type)
  {
  case TYPE_NULL:
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
  case TYPE_ARRAY:
    name = "array";
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
  case TYPE_NULL:
    printf("null");
    break;
  case TYPE_BOOLEAN:
    printf("%s", val.as_boolean ? "true" : "false");
    break;
  case TYPE_NUMBER:
    printf("%g", val.as_number);
    break;
  case TYPE_STRING:
    printf(quoted ? "'%s'" : "%s", AS_STRING(val)->chars);
    break;
  case TYPE_ARRAY:
    array_print(AS_ARRAY(val));
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
  case TYPE_NULL:
    break;
  case TYPE_BOOLEAN:
    result = val1.as_boolean == val2.as_boolean;
    break;
  case TYPE_NUMBER:
    result = val1.as_number == val2.as_number;
    break;
  case TYPE_STRING:
    result = !string_compare(AS_STRING(val1), AS_STRING(val2));
    break;
  case TYPE_ARRAY:
    result = array_equal(AS_ARRAY(val1), AS_ARRAY(val2));
    break;
  }
  return result;
}

int value_compare(value_t val1, value_t val2)
{
  if (val1.type != val2.type)
    fatal_error("cannot compare %s and %s", type_name(val1.type), type_name(val2.type));
  int result = 0;
  switch (val1.type)
  {
  case TYPE_NULL:
    break;
  case TYPE_BOOLEAN:
    result = val1.as_boolean - val2.as_boolean;
    break;
  case TYPE_NUMBER:
    if (val1.as_number > val2.as_number)
    {
      result = 1;
      break;
    }
    if (val1.as_number < val2.as_number)
    {
      result = -1;
      break;
    }
    break;
  case TYPE_STRING:
    result = string_compare(AS_STRING(val1), AS_STRING(val2));
    break;
  case TYPE_ARRAY:
    fatal_error("cannot compare arrays");
    break;
  }
  return result;
}
