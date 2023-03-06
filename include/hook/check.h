//
// The Hook Programming Language
// check.h
//

#ifndef HK_CHECK_H
#define HK_CHECK_H

#include <hook/value.h>

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

#endif // HK_CHECK_H
