//
// The Hook Programming Language
// check.c
//

#include <hook/check.h>
#include <stdlib.h>
#include <hook/status.h>
#include <hook/error.h>
#include <hook/utils.h>

static inline void type_error(int index, int numTypes, HkType types[], HkType val_type);

static inline void type_error(int index, int numTypes, HkType types[], HkType val_type)
{
  hk_assert(numTypes > 0, "numTypes must be greater than 0");
  fprintf(stderr, "runtime error: type error: argument #%d must be of the type %s",
    index, hk_type_name(types[0]));
  for (int i = 1; i < numTypes; ++i)
    fprintf(stderr, "|%s", hk_type_name(types[i]));
  fprintf(stderr, ", %s given\n", hk_type_name(val_type));
}

int hk_check_argument_type(HkValue *args, int index, HkType type)
{
  HkType val_type = args[index].type;
  if (val_type != type)
  {
    hk_runtime_error("type error: argument #%d must be of the type %s, %s given", index,
      hk_type_name(type), hk_type_name(val_type));
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_check_argument_types(HkValue *args, int index, int numTypes, HkType types[])
{
  HkType val_type = args[index].type;
  bool match = false;
  for (int i = 0; i < numTypes; ++i)
    if ((match = (val_type == types[i])))
      break;
  if (!match)
  {
    type_error(index, numTypes, types, val_type);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_check_argument_bool(HkValue *args, int index)
{
  return hk_check_argument_type(args, index, HK_TYPE_BOOL);
}

int hk_check_argument_number(HkValue *args, int index)
{
  return hk_check_argument_type(args, index, HK_TYPE_NUMBER);
}

int hk_check_argument_int(HkValue *args, int index)
{
  HkValue val = args[index];
  if (!hk_is_int(val))
  {
    hk_runtime_error("type error: argument #%d must be of the type int, %s given",
      index, hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int hk_check_argument_string(HkValue *args, int index)
{
  return hk_check_argument_type(args, index, HK_TYPE_STRING);
}

int hk_check_argument_range(HkValue *args, int index)
{
  return hk_check_argument_type(args, index, HK_TYPE_RANGE);
}

int hk_check_argument_array(HkValue *args, int index)
{
  return hk_check_argument_type(args, index, HK_TYPE_ARRAY);
}

int hk_check_argument_struct(HkValue *args, int index)
{
  return hk_check_argument_type(args, index, HK_TYPE_STRUCT);
}

int hk_check_argument_instance(HkValue *args, int index)
{
  return hk_check_argument_type(args, index, HK_TYPE_INSTANCE);
}

int hk_check_argument_iterator(HkValue *args, int index)
{
  return hk_check_argument_type(args, index, HK_TYPE_ITERATOR);
}

int hk_check_argument_callable(HkValue *args, int index)
{
  return hk_check_argument_type(args, index, HK_TYPE_CALLABLE);
}

int hk_check_argument_userdata(HkValue *args, int index)
{
  return hk_check_argument_type(args, index, HK_TYPE_USERDATA);
}
