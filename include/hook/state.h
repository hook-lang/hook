//
// The Hook Programming Language
// state.h
//

#ifndef HK_STATE_H
#define HK_STATE_H

#include <hook/range.h>
#include <hook/struct.h>
#include <hook/callable.h>
#include <hook/userdata.h>

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
int32_t hk_state_compare(hk_value_t val1, hk_value_t val2, int32_t *result);

#endif // HK_STATE_H
