//
// The Hook Programming Language
// check.h
//

#ifndef HK_CHECK_H
#define HK_CHECK_H

#include <hook/value.h>

int hk_check_argument_type(HkValue *args, int index, HkType type);
int hk_check_argument_types(HkValue *args, int index, int numTypes, HkType types[]);
int hk_check_argument_bool(HkValue *args, int index);
int hk_check_argument_number(HkValue *args, int index);
int hk_check_argument_int(HkValue *args, int index);
int hk_check_argument_string(HkValue *args, int index);
int hk_check_argument_range(HkValue *args, int index);
int hk_check_argument_array(HkValue *args, int index);
int hk_check_argument_struct(HkValue *args, int index);
int hk_check_argument_instance(HkValue *args, int index);
int hk_check_argument_iterator(HkValue *args, int index);
int hk_check_argument_callable(HkValue *args, int index);
int hk_check_argument_userdata(HkValue *args, int index);

#endif // HK_CHECK_H
