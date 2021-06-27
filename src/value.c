//
// Hook Programming Language
// value.c
//

#include "value.h"
#include "string.h"
#include "array.h"

static inline void free_value(value_t val);

static inline void free_value(value_t val)
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

void value_free(value_t val)
{
  if (IS_OBJECT(val) && IS_UNREACHABLE(AS_OBJECT(val)))
    free_value(val);
}

void value_release(value_t val)
{
  if (!IS_OBJECT(val))
    return;
  object_t *obj = AS_OBJECT(val);
  DECR_REF(obj);
  if (IS_UNREACHABLE(obj))
    free_value(val);
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
