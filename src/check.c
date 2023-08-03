//
// The Hook Programming Language
// check.c
//

#include <hook/check.h>
#include <stdlib.h>
#include <hook/error.h>
#include <hook/utils.h>

static inline void type_error(HkState *state, int index, int numTypes, HkType types[], HkType valType);

static inline void type_error(HkState *state, int index, int numTypes, HkType types[], HkType valType)
{
  hk_assert(numTypes > 0, "numTypes must be greater than 0");
  state->status = HK_STATE_STATUS_ERROR;
  fprintf(stderr, "runtime error: type error: argument #%d must be of the type %s",
    index, hk_type_name(types[0]));
  for (int i = 1; i < numTypes; ++i)
    fprintf(stderr, "|%s", hk_type_name(types[i]));
  fprintf(stderr, ", %s given\n", hk_type_name(valType));
}

void hk_state_check_argument_type(HkState *state, HkValue *args, int index, HkType type)
{
  HkType valType = args[index].type;
  if (valType != type)
    hk_state_error(state, "type error: argument #%d must be of the type %s, %s given", index,
      hk_type_name(type), hk_type_name(valType));
}

void hk_state_check_argument_types(HkState *state, HkValue *args, int index, int numTypes, HkType types[])
{
  HkType valType = args[index].type;
  bool match = false;
  for (int i = 0; i < numTypes; ++i)
    if ((match = (valType == types[i])))
      break;
  if (!match)
    type_error(state, index, numTypes, types, valType);
}

void hk_state_check_argument_bool(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_BOOL);
}

void hk_state_check_argument_number(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_NUMBER);
}

void hk_state_check_argument_int(HkState *state, HkValue *args, int index)
{
  HkValue val = args[index];
  if (!hk_is_int(val))
    hk_state_error(state, "type error: argument #%d must be of the type int, %s given",
      index, hk_type_name(val.type));
}

void hk_state_check_argument_string(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_STRING);
}

void hk_state_check_argument_range(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_RANGE);
}

void hk_state_check_argument_array(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_ARRAY);
}

void hk_state_check_argument_struct(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_STRUCT);
}

void hk_state_check_argument_instance(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_INSTANCE);
}

void hk_state_check_argument_iterator(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_ITERATOR);
}

void hk_state_check_argument_callable(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_CALLABLE);
}

void hk_state_check_argument_userdata(HkState *state, HkValue *args, int index)
{
  hk_state_check_argument_type(state, args, index, HK_TYPE_USERDATA);
}
