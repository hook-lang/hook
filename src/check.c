//
// The Hook Programming Language
// check.c
//

#include <hook/check.h>
#include <stdlib.h>
#include <hook/status.h>
#include <hook/error.h>
#include <hook/utils.h>

static inline void type_error(int32_t index, int32_t num_types, hk_type_t types[], hk_type_t val_type);

static inline void type_error(int32_t index, int32_t num_types, hk_type_t types[], hk_type_t val_type)
{
  hk_assert(num_types > 0, "num_types must be greater than 0");
  fprintf(stderr, "runtime error: type error: argument #%d must be of the type %s",
    index, hk_type_name(types[0]));
  for (int32_t i = 1; i < num_types; ++i)
    fprintf(stderr, "|%s", hk_type_name(types[i]));
  fprintf(stderr, ", %s given\n", hk_type_name(val_type));
}

int32_t hk_check_argument_type(hk_value_t *args, int32_t index, hk_type_t type)
{
  hk_type_t val_type = args[index].type;
  if (val_type != type)
  {
    hk_runtime_error("type error: argument #%d must be of the type %s, %s given", index,
      hk_type_name(type), hk_type_name(val_type));
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int32_t hk_check_argument_types(hk_value_t *args, int32_t index, int32_t num_types, hk_type_t types[])
{
  hk_type_t val_type = args[index].type;
  bool match = false;
  for (int32_t i = 0; i < num_types; ++i)
    if ((match = (val_type == types[i])))
      break;
  if (!match)
  {
    type_error(index, num_types, types, val_type);
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int32_t hk_check_argument_bool(hk_value_t *args, int32_t index)
{
  return hk_check_argument_type(args, index, HK_TYPE_BOOL);
}

int32_t hk_check_argument_number(hk_value_t *args, int32_t index)
{
  return hk_check_argument_type(args, index, HK_TYPE_NUMBER);
}

int32_t hk_check_argument_int(hk_value_t *args, int32_t index)
{
  hk_value_t val = args[index];
  if (!hk_is_int(val))
  {
    hk_runtime_error("type error: argument #%d must be of the type int, %s given",
      index, hk_type_name(val.type));
    return HK_STATUS_ERROR;
  }
  return HK_STATUS_OK;
}

int32_t hk_check_argument_string(hk_value_t *args, int32_t index)
{
  return hk_check_argument_type(args, index, HK_TYPE_STRING);
}

int32_t hk_check_argument_range(hk_value_t *args, int32_t index)
{
  return hk_check_argument_type(args, index, HK_TYPE_RANGE);
}

int32_t hk_check_argument_array(hk_value_t *args, int32_t index)
{
  return hk_check_argument_type(args, index, HK_TYPE_ARRAY);
}

int32_t hk_check_argument_struct(hk_value_t *args, int32_t index)
{
  return hk_check_argument_type(args, index, HK_TYPE_STRUCT);
}

int32_t hk_check_argument_instance(hk_value_t *args, int32_t index)
{
  return hk_check_argument_type(args, index, HK_TYPE_INSTANCE);
}

int32_t hk_check_argument_iterator(hk_value_t *args, int32_t index)
{
  return hk_check_argument_type(args, index, HK_TYPE_ITERATOR);
}

int32_t hk_check_argument_callable(hk_value_t *args, int32_t index)
{
  return hk_check_argument_type(args, index, HK_TYPE_CALLABLE);
}

int32_t hk_check_argument_userdata(hk_value_t *args, int32_t index)
{
  return hk_check_argument_type(args, index, HK_TYPE_USERDATA);
}
