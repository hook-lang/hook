//
// userdata.c
//
// Copyright 2021 The Hook Programming Language Authors.
//
// This file is part of the Hook project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include <hook/userdata.h>
#include <stdlib.h>

void hk_userdata_init(HkUserdata *udata, void (*deinit)(HkUserdata *))
{
  udata->refCount = 0;
  udata->deinit = deinit;
}

void hk_userdata_free(HkUserdata *udata)
{
  if (udata->deinit)
    udata->deinit(udata);
  free(udata);
}
