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
  int stackEnd;
  int stackTop;
  HkValue *stackSlots;
} HkState;

void hk_state_init(HkState *state, int minCapacity);
void hk_state_free(HkState *state);
int hk_state_push(HkState *state, HkValue val);
int hk_state_push_nil(HkState *state);
int hk_state_push_bool(HkState *state, bool data);
int hk_state_push_number(HkState *state, double data);
int hk_state_push_string(HkState *state, HkString *str);
int hk_state_push_string_from_chars(HkState *state, int length, const char *chars);
int hk_state_push_string_from_stream(HkState *state, FILE *stream, const char terminal);
int hk_state_push_range(HkState *state, HkRange *range);
int hk_state_push_array(HkState *state, HkArray *arr);
int hk_state_push_struct(HkState *state, HkStruct *ztruct);
int hk_state_push_instance(HkState *state, HkInstance *inst);
int hk_state_push_iterator(HkState *state, HkIterator *it);
int hk_state_push_closure(HkState *state, HkClosure *cl);
int hk_state_push_native(HkState *state, HkNative *native);
int hk_state_push_new_native(HkState *state, const char *name, int arity, int (*call)(HkState *, HkValue *));
int hk_state_push_userdata(HkState *state, HkUserdata *udata);
int hk_state_array(HkState *state, int length);
int hk_state_struct(HkState *state, int length);
int hk_state_instance(HkState *state, int num_args);
int hk_state_construct(HkState *state, int length);
void hk_state_pop(HkState *state);
int hk_state_call(HkState *state, int num_args);
int hk_state_compare(HkValue val1, HkValue val2, int *result);

#endif // HK_STATE_H
