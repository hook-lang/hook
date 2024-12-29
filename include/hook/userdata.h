//
// userdata.h
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
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
