//
// The Hook Programming Language
// hk_state.h
//

#ifndef HK_STATE_H
#define HK_STATE_H

#include "hk_range.h"
#include "hk_struct.h"
#include "hk_callable.h"
#include "hk_userdata.h"

#define HK_STACK_MIN_CAPACITY (1 << 8)

typedef struct hk_state
{
  int32_t stack_end;
  int32_t stack_top;
  hk_value_t *stack;
} hk_state_t;

void hk_state_init(hk_state_t *state, int32_t min_capacity);
void hk_state_free(hk_state_t *state);
int32_t hk_state_push(hk_state_t *state, hk_value_t val);
int32_t hk_state_push_nil(hk_state_t *state);
int32_t hk_state_push_bool(hk_state_t *state, bool data);
int32_t hk_state_push_number(hk_state_t *state, double data);
int32_t hk_state_push_string(hk_state_t *state, hk_string_t *str);
int32_t hk_state_push_string_from_chars(hk_state_t *state, int32_t length, const char *chars);
int32_t hk_state_push_string_from_stream(hk_state_t *state, FILE *stream, const char terminal);
int32_t hk_state_push_range(hk_state_t *state, hk_range_t *range);
int32_t hk_state_push_array(hk_state_t *state, hk_array_t *arr);
int32_t hk_state_push_struct(hk_state_t *state, hk_struct_t *ztruct);
int32_t hk_state_push_instance(hk_state_t *state, hk_instance_t *inst);
int32_t hk_state_push_iterator(hk_state_t *state, hk_iterator_t *it);
int32_t hk_state_push_closure(hk_state_t *state, hk_closure_t *cl);
int32_t hk_state_push_native(hk_state_t *state, hk_native_t *native);
int32_t hk_state_push_new_native(hk_state_t *state, const char *name, int32_t arity, int32_t (*call)(hk_state_t *, hk_value_t *));
int32_t hk_state_push_userdata(hk_state_t *state, hk_userdata_t *udata);
int32_t hk_state_array(hk_state_t *state, int32_t length);
int32_t hk_state_struct(hk_state_t *state, int32_t length);
int32_t hk_state_instance(hk_state_t *state, int32_t num_args);
int32_t hk_state_construct(hk_state_t *state, int32_t length);
void hk_state_pop(hk_state_t *state);
int32_t hk_state_call(hk_state_t *state, int32_t num_args);
int32_t hk_state_compare(hk_state_t *state, hk_value_t val1, hk_value_t val2, int32_t *result);
int32_t hk_check_argument_type(hk_value_t *args, int32_t index, hk_type_t type);
int32_t hk_check_argument_types(hk_value_t *args, int32_t index, int32_t num_types, hk_type_t types[]);
int32_t hk_check_argument_bool(hk_value_t *args, int32_t index);
int32_t hk_check_argument_number(hk_value_t *args, int32_t index);
int32_t hk_check_argument_int(hk_value_t *args, int32_t index);
int32_t hk_check_argument_string(hk_value_t *args, int32_t index);
int32_t hk_check_argument_range(hk_value_t *args, int32_t index);
int32_t hk_check_argument_array(hk_value_t *args, int32_t index);
int32_t hk_check_argument_struct(hk_value_t *args, int32_t index);
int32_t hk_check_argument_instance(hk_value_t *args, int32_t index);
int32_t hk_check_argument_iterator(hk_value_t *args, int32_t index);
int32_t hk_check_argument_callable(hk_value_t *args, int32_t index);
int32_t hk_check_argument_userdata(hk_value_t *args, int32_t index);

#endif // HK_STATE_H
