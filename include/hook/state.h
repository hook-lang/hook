//
// The Hook Programming Language
// state.h
//

#ifndef HK_STATE_H
#define HK_STATE_H

#include "callable.h"
#include "range.h"
#include "struct.h"
#include "userdata.h"

#define HK_STATE_FLAG_NONE     0x00
#define HK_STATE_FLAG_NO_TRACE 0x01

#define HK_STACK_MIN_CAPACITY (1 << 8)

#define hk_state_is_no_trace(s) ((s)->flags & HK_STATE_FLAG_NO_TRACE)

#define hk_state_is_ok(s)    ((s)->status == HK_STATE_STATUS_OK)
#define hk_state_is_exit(s)  ((s)->status == HK_STATE_STATUS_EXIT)
#define hk_state_is_error(s) ((s)->status == HK_STATE_STATUS_ERROR)

#define hk_return_if_not_ok(s) do \
  { \
    if (!hk_state_is_ok(s)) \
      return; \
  } while (0)

typedef enum
{
  HK_STATE_STATUS_OK,
  HK_STATE_STATUS_EXIT,
  HK_STATE_STATUS_ERROR
} HkSateStatus;

typedef struct hk_state
{
  int stackEnd;
  int stackTop;
  HkValue *stackSlots;
  int flags;
  HkSateStatus status;
} HkState;

void hk_state_init(HkState *state, int minCapacity);
void hk_state_deinit(HkState *state);
void hk_state_runtime_error(HkState *state, const char *fmt, ...);
void hk_state_check_argument_type(HkState *state, HkValue *args, int index, HkType type);
void hk_state_check_argument_types(HkState *state, HkValue *args, int index, int numTypes, HkType types[]);
void hk_state_check_argument_bool(HkState *state, HkValue *args, int index);
void hk_state_check_argument_number(HkState *state, HkValue *args, int index);
void hk_state_check_argument_int(HkState *state, HkValue *args, int index);
void hk_state_check_argument_string(HkState *state, HkValue *args, int index);
void hk_state_check_argument_range(HkState *state, HkValue *args, int index);
void hk_state_check_argument_array(HkState *state, HkValue *args, int index);
void hk_state_check_argument_struct(HkState *state, HkValue *args, int index);
void hk_state_check_argument_instance(HkState *state, HkValue *args, int index);
void hk_state_check_argument_iterator(HkState *state, HkValue *args, int index);
void hk_state_check_argument_callable(HkState *state, HkValue *args, int index);
void hk_state_check_argument_userdata(HkState *state, HkValue *args, int index);
void hk_state_push(HkState *state, HkValue val);
void hk_state_push_nil(HkState *state);
void hk_state_push_bool(HkState *state, bool data);
void hk_state_push_number(HkState *state, double data);
void hk_state_push_string(HkState *state, HkString *str);
void hk_state_push_string_from_chars(HkState *state, int length, const char *chars);
void hk_state_push_string_from_stream(HkState *state, FILE *stream, const char delim);
void hk_state_push_range(HkState *state, HkRange *range);
void hk_state_push_array(HkState *state, HkArray *arr);
void hk_state_push_struct(HkState *state, HkStruct *ztruct);
void hk_state_push_instance(HkState *state, HkInstance *inst);
void hk_state_push_iterator(HkState *state, HkIterator *it);
void hk_state_push_closure(HkState *state, HkClosure *cl);
void hk_state_push_native(HkState *state, HkNative *native);
void hk_state_push_new_native(HkState *state, const char *name, int arity, void (*call)(HkState *, HkValue *));
void hk_state_push_userdata(HkState *state, HkUserdata *udata);
void hk_state_array(HkState *state, int length);
void hk_state_struct(HkState *state, int length);
void hk_state_instance(HkState *state, int num_args);
void hk_state_construct(HkState *state, int length);
void hk_state_pop(HkState *state);
void hk_state_call(HkState *state, int num_args);
void hk_state_compare(HkState *state, HkValue val1, HkValue val2, int *result);

#endif // HK_STATE_H
