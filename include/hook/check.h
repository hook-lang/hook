//
// The Hook Programming Language
// check.h
//

#ifndef HK_CHECK_H
#define HK_CHECK_H

#include <hook/state.h>

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

#endif // HK_CHECK_H
