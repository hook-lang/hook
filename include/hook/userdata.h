//
// The Hook Programming Language
// userdata.h
//

#ifndef HK_USERDATA_H
#define HK_USERDATA_H

#include "value.h"

#define HK_USERDATA_HEADER HK_OBJECT_HEADER \
                           void (*deinit)(struct HkUserdata *);

typedef struct HkUserdata
{
  HK_USERDATA_HEADER
} HkUserdata;

void hk_userdata_init(HkUserdata *udata, void (*deinit)(HkUserdata *));
void hk_userdata_free(HkUserdata *udata);

#endif // HK_USERDATA_H
