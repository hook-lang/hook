//
// The Hook Programming Language
// hk_userdata.h
//

#ifndef HK_USERDATA_H
#define HK_USERDATA_H

#include "hk_value.h"

#define HK_USERDATA_HEADER HK_OBJECT_HEADER \
                             void (*deinit)(struct hk_userdata *);

typedef struct hk_userdata
{
  HK_USERDATA_HEADER
} hk_userdata_t;

void hk_userdata_init(hk_userdata_t *udata, void (*deinit)(struct hk_userdata *));
void hk_userdata_free(hk_userdata_t *udata);

#endif // HK_USERDATA_H
