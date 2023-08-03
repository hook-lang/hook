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
void hk_state_free(HkState *state);
void hk_state_error(HkState *state, const char *fmt, ...);
void hk_state_push(HkState *state, HkValue val);
void hk_state_push_nil(HkState *state);
void hk_state_push_bool(HkState *state, bool data);
void hk_state_push_number(HkState *state, double data);
void hk_state_push_string(HkState *state, HkString *str);
void hk_state_push_string_from_chars(HkState *state, int length, const char *chars);
void hk_state_push_string_from_stream(HkState *state, FILE *stream, const char terminal);
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
