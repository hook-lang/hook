//
// Hook Programming Language
// value.c
//

#include "value.h"
#include "string.h"

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
