//
// Hook Programming Language
// value.c
//

#include "value.h"

const char *type_name(type_t type)
{
  char *name = "null";
  switch (type)
  {
  case TYPE_NUMBER:
    name = "number";
    break;
  default:
    break;
  }
  return name;
}
